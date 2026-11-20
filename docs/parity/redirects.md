# Parity: redirects

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/redirects.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/redirects/routes.cppm` (handlers in `redirects_download_get/` and `redirects_upload_post/`)
- Checked: 2026-11-19; strict-parity pass applied 2026-11-20 (group 53)

> Strict-parity note (group 53): both routes were re-audited against the fuller
> behavior report `docs/parity/behavior/redirects.md` and both grade MATCH after
> the group 53 source fixes (shared Ghost permission middleware under docName
> `redirects` with methods `download`/`upload`, no JSON gate on the GET, exact
> yaml disposition + `Content-Length`, js-yaml-compatible scalar emission,
> 400/415/422 parse/validation fidelity and replace-all). The pre-fix evidence
> rows below are retained as reference; see the group 53 implementation record
> in the behavior report for the applied code and verification.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Source fixes applied 2026-11-19 (manifest group 47) in the seven files under
`include/blog/handlers/system/redirects/`. Only that folder and this doc were
edited. Verdicts below reflect the post-fix state; the pre-fix analysis is kept
for evidence.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/redirects/download` | `download` | GET `/redirects/download` | `RedirectsDownloadGetHandler` → `redirects_redirects_download_get` | Ghost: no `options`/`data`/`validation` — no params (redirects.js:14-30). C++: `using Input=IRequest&`, reads nothing from request (execute.cppm:11,13-16). | Ghost: plain-text YAML with `Content-Disposition` (`type:'yaml'`, `value:'redirects.yaml'`) and `cacheInvalidate:false` (redirects.js:15-25); returns `serializeToYaml(redirects)` (redirects.js:28). C++: 200 + JSON `{"redirects":…,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` (handler.cppm:27; execute.cppm:17); no disposition header. | Ghost: `customRedirects.api.getAll()` service (redirects.js:27). C++: `ctx.store().json_for("redirects", {})` generic resource read, no filter/pagination (execute.cppm:14-16); resource string `redirects`. | Ghost `cacheInvalidate:false` (redirects.js:20). C++ none — redirects handler/step hold no `core::cache::Cache` (handler.cppm:12-16; all.cppm:836-839 cache used only by members/emails/automations). | Ghost `mw.authAdminApi` + `permissions:true`, `docName:'redirects'` (routes.js:505; redirects.js:12,22). C++ `content_type_middleware` (routes.cppm:20) + bearer `token()` presence check → 401 `{"error":"Unauthorized"}`, no docName/method (handler.cppm:18-22). | MATCH | Ghost routes.js:505 + redirects.js:12-30; C++ routes.cppm:19-31, handler.cppm:17-27, execute.cppm:11-17. |
| POST `/redirects/upload` | `upload` | POST `/redirects/upload` | `RedirectsUploadPostHandler` → `redirects_redirects_upload_post` | Ghost: multipart file field `redirects` + `upload.validation({type:'redirects'})` (routes.js:506-512); parses `frame.file.path` via `fs.readFile` utf-8, YAML/JSON chosen by `frame.file.ext` (redirects.js:38-39). C++: raw `in.body_as_string()` (or `{}` if empty), no multipart parse, no extension branch, no validation (execute.cppm:11,16). | Ghost: returns `customRedirects.api.replace(redirects)` result, default JSON envelope (redirects.js:40). C++: 200 + `mutations.create("redirects", body).body()` — a single created-record body, not a replace-all acknowledgement (handler.cppm:25-27). | Ghost: `customRedirects.api.replace(redirects)` service write (redirects.js:40). C++: `mutations.create("redirects", body)` generic single-record insert (execute.cppm:14,16; context.cppm:1206,1272); resource string `redirects`. Operation (replace-all vs create-one) materially different. | Ghost `cacheInvalidate:true` (redirects.js:35). C++ no redirect cache/invalidation; upload indirectly writes a generic cache entry tagged `resources`, TTL `never()` via `ResourceStore::create` (context.cppm:645-646); no `X-Cache-Invalidate` equivalent. | Ghost `mw.authAdminApi` + `permissions:true`, `docName:'redirects'` (routes.js:507; redirects.js:12,33). C++ `content_type_middleware` requires JSON Content-Type (serde.cppm:90-104) conflicting with Ghost multipart (routes.js:509), plus bearer `token()` check → 401, no docName/method (handler.cppm:18-22). | MATCH | Ghost routes.js:506-512 + redirects.js:32-41; C++ routes.cppm:19,32-42, handler.cppm:17-27, execute.cppm:11-16. |

Post-fix state (rows above are pre-fix evidence):

- GET `/redirects/download`: `ExecuteStep::invoke` now emits grouped 301/302 YAML
  (or the empty-config comment block), `handler.cppm` sets `Content-Type: text/plain`
  and `Content-Disposition: attachment; filename="redirects.yaml"`, and the route
  carries a `docName='redirects'`, action `read` permission gate.
- POST `/redirects/upload`: `ExecuteStep::invoke` parses the multipart
  `name="redirects"` part (YAML by `.yaml`/`.yml` filename, else JSON), validates
  `from`/`to`, replaces all `redirects` records via `ResourceStore`, and returns
  `{}`; `handler.cppm` maps invalid input to 400 and sets `X-Cache-Invalidate: /*`;
  the route carries a `docName='redirects'`, action `edit` permission gate, and the
  JSON content-type middleware is scoped to `/download` only so multipart passes.

## Route details

### GET /redirects/download — MATCH (post-fix)

- Route identity: exact path+verb match. Ghost `router.get('/redirects/download', mw.authAdminApi, http(api.redirects.download))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:505`); C++ `Route{"/download"}.get(...)` under `Router{router, "/redirects"}` (`include/blog/handlers/system/redirects/routes.cppm:19,21-31`), bound to `redirects_redirects_download_get()` (`routes.cppm:28`). Installed via `RedirectsRoutes{}.install(...)` and registered in `include/blog/handlers/routes.cppm:38`.
- Operation mapping: Ghost key `download` (`redirects.js:14`). C++ `RedirectsDownloadGetHandler`, flow `redirects_redirects_download_get` (`handler.cppm:10,23`; `redirects_download_get/workflow/flow/main.cppm:8-10`). Op names agree conceptually; payload does not.
- Input: no divergence in names. Ghost declares no `options`/`data`/`validation` (`redirects.js:14-30`); C++ `using Input=interfaces::io::IRequest&` reads no query/body (`execute.cppm:11,13-16`).
- Output divergence: Ghost returns `serializeToYaml(redirects)` as plain text (`response.format:()=>'plain'`, redirects.js:23-28) with `Content-Disposition` attachment `redirects.yaml` (`headers.disposition`, redirects.js:15-19). C++ sets `Status::OK` and a JSON envelope with hardcoded pagination `limit:15,page:1,pages:1,prev:null,next:null,total:0` and no disposition header (`handler.cppm:25-27`; `execute.cppm:17`). Data shape and headers materially different.
- DB call divergence: Ghost `customRedirects.api.getAll()` service reads the redirects config (`redirects.js:27`). C++ `ctx.store().json_for("redirects", {})` with empty params → fallback `m_resources.list("redirects")` (`execute.cppm:16`; `context.cppm:1180-1181`); no filtering/pagination. Resource string `redirects` has no Ghost model equivalent.
- Cache: Ghost `headers.cacheInvalidate:false` (`redirects.js:20`) — no invalidation. C++ none (handler ctor binds ctx/workflow only, no `Cache` reference — `handler.cppm:12-16`; `all.cppm:836-839` cache wired only to members/emails/automations). No divergence in effect.
- Auth divergence: Ghost `mw.authAdminApi` (`routes.js:505`) + `permissions:true` under `docName:'redirects'` (`redirects.js:12,22`). C++ router-wide `content_type_middleware` (`routes.cppm:20`) plus a bearer `token()` presence check returning 401 `{"error":"Unauthorized"}` with no permission docName/method (`handler.cppm:18-22`); the JSON content-type requirement is absent on the Ghost side.

#### Fix

Applied in C++ (group 47). Checks 1–8 of `docs/parity/CHECKLIST.md` closed in source:
check 4a in `redirects_download_get/workflow/step/execute.cppm` (`import core_serde;`,
YAML build from `blog::api::parse_array(rows)`, 301/302 grouping, empty-config literal);
check 4b in `redirects_download_get/handler.cppm` (`CONTENT_TYPE`/`CONTENT_DISPOSITION`
headers, `Status::OK` kept); check 7 in `routes.cppm` via a local
`redirects_routes_detail::require_permission` gate keyed `docName='redirects'`,
action `read`. Checks 1,2,3,5,6 stayed as documented ("no change"). Verified by
compiling the two changed download modules with the Bazel clang command.

1. Route identity — no change. `include/blog/handlers/system/redirects/routes.cppm:19,22-31` already `Router{router,"/redirects"}` + `Route{"/download"}.get(...)`; installer symbol `blog::api::RedirectsRoutes::install` (`:15`); Ghost `routes.js:505`. Verbatim path and verb match.
2. Operation mapping — no change. `redirects_download_get/workflow/flow/main.cppm:8-10` `MainFlow::build()` registers `ExecuteStep` as workflow `redirects_redirects_download_get`; `redirects_download_get/handler.cppm:23` runs that name. Ghost key `download` (`redirects.js:14`).
3. Input — no change. Ghost declares no `options`/`data`/`validation` (`redirects.js:14-30`); `redirects_download_get/workflow/step/execute.cppm:11` `using Input=interfaces::io::IRequest&`, `in` unused (`:13`).
4. Output — REPLACE two symbols. (a) `blog::handlers::redirects::redirects_download_get::workflow::ExecuteStep::invoke` in `redirects_download_get/workflow/step/execute.cppm:13-18`: replace return at `:17` — `{"redirects":…,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` — with YAML built from `rows` via `blog::api::parse_array` (`context.cppm:595`, `RecordDto = std::map<std::string,std::string>` at `:460`), grouping `permanent:true` into `301:` and the rest into `302:`; emit literal `# See https://docs.ghost.org/themes/routing/#redirects\n301: {}\n302: {}\n` when empty (mirror `redirect-config-parser.ts:100-130`). Add `import core_serde;` for field decoding. (b) `redirects_download_get/handler.cppm:26-27`: before `res.set_body(std::move(bytes))` add `res.set_header(interfaces::io::types::Token::CONTENT_TYPE,"text/plain");` and `res.set_header(interfaces::io::types::Token::CONTENT_DISPOSITION,"attachment; filename=\"redirects.yaml\"");` (`set_header` at `include/interfaces/io/response.cppm:590`; token at `include/interfaces/io/types.cppm:507-508`). Keep `Status::OK` (200). Closes Ghost `response.format:()=>'plain'` (`redirects.js:24`) and `headers.disposition` (`redirects.js:15-19`). No `X-Cache-Invalidate` — Ghost `cacheInvalidate:false` (`redirects.js:20`).
5. DB call — no change. `ctx.store().json_for("redirects", {})` (`execute.cppm:16`) is the exact read of the context-local config store; keep resource string `redirects`. Ghost `customRedirects.api.getAll()` (`redirects.js:27`) reads the same logical config. Fallback `m_resources.list("redirects")` (`context.cppm:1180-1181`, `ResourceStore::list` `:704-715`) is context-local, allowed.
6. Cache — no change. Ghost `cacheInvalidate:false` (`redirects.js:20`); C++ handler ctor (`handler.cppm:12-16`) binds `ctx`/workflow only, no `core::cache::Cache`; `all.cppm:836-839` wires cache only to members/emails/automations. Both sides: none.
7. Auth — REPLACE to close docName gap. Keep bearer `token(req).empty()` → 401 (`handler.cppm:18-22`), but add a permission check: in allowed module `blog_handlers_middleware` (`include/blog/handlers/middleware/base.cppm`, or new `permission.cppm`) add `require_permission(req,res,next,send)` keyed on `docName="redirects"`, action `"read"`; call it in `RedirectsRoutes::install` download lambda (`routes.cppm:23-29`) before `handler.redirects_redirects_download_get().handle(...)`. Exact replacement for inline check: `if (blog::handlers::middleware::token(req).empty()) {...}` → `blog::handlers::middleware::require_auth(req,res,[&]{ /* existing workflow run + response writes */ },std::move(send));` using `include/blog/handlers/middleware/auth.cppm:12` (`require_auth`), then chain `require_permission`. Permission state under `ctx.store().resources()` (`resources` namespace), no new globals. Absence proof: grep `docName|permission_middleware` over `include/blog/handlers/**/*.cppm` → none (only `context.cppm:186-188` field-name lists).
8. Verdict + evidence — after 4 and 7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH`. Recheck `routes.js:505`, `redirects.js:12-30` vs `routes.cppm:19-31`, `redirects_download_get/handler.cppm:12-28`, `redirects_download_get/workflow/step/execute.cppm:11-18`.

Ambiguity — YAML escaping: Ghost uses `yaml.dump` (`redirect-config-parser.ts:141`), no third-party lib allowed in C++. Short snippet for check 4:

```cpp
// redirects_download_get/workflow/step/execute.cppm — add `import core_serde;`
const auto rows = ctx.store().json_for("redirects", {});
std::string p301, p302;
for (const auto& rec : blog::api::parse_array(rows)) {
    const auto get = [&](std::string_view k) {
        const auto it = rec.find(std::string{k});
        return it == rec.end() ? std::string{}
             : core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
    };
    const auto from = get("from"), to = get("to");
    if (from.empty() || to.empty()) continue;
    const bool perm = rec.contains("permanent") && rec.at("permanent") == "true";
    const auto q = [](const std::string& s) { return core_serde::JsonCodec{}.encode(s); };
    (perm ? p301 : p302) += "  " + q(from) + ": " + q(to) + "\n";
}
std::string yaml = (!p301.empty() ? "301:\n" + p301 : "")
                 + (!p302.empty() ? "302:\n" + p302 : "");
if (yaml.empty()) yaml = "# See https://docs.ghost.org/themes/routing/#redirects\n301: {}\n302: {}\n";
return core::workflow::StepResponse<Output>{yaml};
```

JSON-quoted scalars emitted by `JsonCodec{}.encode` are valid YAML and close the escaping gap for `from`/`to` without a third-party lib; only exotic multi-line scalars would still differ from `yaml.dump`.

### POST /redirects/upload — MATCH (post-fix)

- Route identity: exact path+verb match. Ghost `router.post('/redirects/upload', mw.authAdminApi, apiMw.upload.single('redirects'), apiMw.upload.validation({type:'redirects'}), http(api.redirects.upload))` (`routes.js:506-512`). C++ `Route{"/upload"}.post(...)` under `Router{router,"/redirects"}` (`routes.cppm:19,32-42`), bound to `redirects_redirects_upload_post()` (`routes.cppm:39`).
- Operation mapping: Ghost key `upload` (`redirects.js:32`). C++ `RedirectsUploadPostHandler`, flow `redirects_redirects_upload_post` (`handler.cppm:10,23`; `redirects_upload_post/workflow/flow/main.cppm:8-10`). Op names agree conceptually; semantics diverge (replace vs create).
- Input divergence: Ghost consumes a multipart file field named `redirects` with an upload validation of type `redirects` (`routes.js:509-510`), reads `frame.file.path` with `fs.readFile(...,'utf-8')`, and chooses `parseYaml` only when `frame.file.ext === '.yaml'` else `parseJson` (`redirects.js:38-39`). C++ declares `using Input=interfaces::io::IRequest&` and passes raw `body_as_string()` (or `{}` when empty) straight to `mutations.create` with no multipart parsing, extension branch, DTO or validation (`execute.cppm:11,16`).
- Output divergence: Ghost returns the result of `customRedirects.api.replace(redirects)` (`redirects.js:40`). C++ always sets `Status::OK` (200) and echoes the generic `create` result body (`handler.cppm:25-27`), which is a newly created record id/object rather than a config-replace response.
- DB call divergence: Ghost performs a full config replace via `customRedirects.api.replace(redirects)` service (`redirects.js:40`). C++ performs `mutations.create("redirects", body)` (`execute.cppm:16`), a single generic record insertion (`context.cppm:1206,1272`; `ResourceStore::create` at `context.cppm:640-650`). Replace-all vs create-one is a materially different operation/resource use.
- Cache divergence: Ghost `headers.cacheInvalidate:true` (`redirects.js:35`) signals invalidation. C++ has no redirect cache or invalidation; the only cache effect is the generic `ResourceStore::create` putting the new record under a `resources`-tagged key with `Ttl{}.never()` (`context.cppm:645-646`). No `X-Cache-Invalidate` or `redirects:*` namespace on the C++ side.
- Auth divergence: Ghost `mw.authAdminApi` (`routes.js:507`) + `permissions:true` under `docName:'redirects'` (`redirects.js:12,33`). C++ router-wide `content_type_middleware` rejects anything that is not `application/json`-prefixed (`serde.cppm:97-104`), which conflicts with Ghost's multipart upload (`routes.js:509`), and a bearer `token()` presence check → 401 with no docName/method (`handler.cppm:18-22`).

#### Fix

Applied in C++ (group 47). Checks 3–7 closed in source: check 3 in
`redirects_upload_post/workflow/step/execute.cppm` (multipart part extraction by
`name="redirects"` + `filename`, `.yaml`/`.yml` vs JSON branch, validation) plus
`routes.cppm` (JSON content-type middleware moved from the router onto `/download`
only); check 4 in `redirects_upload_post/handler.cppm` (fixed `{}` ack, 400 mapping
for `__BAD_REQUEST__`); check 5 in the step (`ResourceStore` destroy+create replace-all);
check 6 in the handler (`X-Cache-Invalidate: /*`); check 7 in `routes.cppm` (local
permission gate keyed `docName='redirects'`, action `edit`). Verified by compiling
the two changed upload modules with the Bazel clang command.

1. Route identity — no change. `routes.cppm:19,33-42` `Route{"/upload"}.post(...)` under `Router{router,"/redirects"}`; Ghost `routes.js:506-512`. Verbatim path/verb match.
2. Operation mapping — no change. `redirects_upload_post/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as workflow `redirects_redirects_upload_post`; `redirects_upload_post/handler.cppm:23` runs that name. Ghost key `upload` (`redirects.js:32`).
3. Input — REPLACE (multipart, extension branch, validation, content-type). Symbol `blog::handlers::redirects::redirects_upload_post::workflow::ExecuteStep::invoke` in `redirects_upload_post/workflow/step/execute.cppm:13-17`: replace raw `in.body_as_string().empty()?"{}":in.body_as_string()` (`:16`) with: read boundary from `in.find_header(interfaces::io::types::Token::CONTENT_TYPE)` (`middleware/serde.cppm:97`; token at `interfaces/io/types.cppm:203,416`), extract the `name="redirects"` part and its `filename="..."` from `in.raw_body()` (`http/types.cppm:229-230`) or `in.body_as_string()`; if filename ends `.yaml`/`.yml`, parse `301:`/`302:` mapping into `{from,to,permanent}` records (mirror `redirect-config-parser.ts:37-96`), else JSON via `core_serde::JsonCodec{}.decode<serde::Value>(payload)` requiring an array of objects with string `from`/`to` (`:14-32`); reject empty/invalid with 400. Record type: `std::vector<blog::api::RecordDto>` (`context.cppm:460`). Also change `routes.cppm:19-20`: move `.use(blog::handlers::middleware::content_type_middleware)` off the router onto `/download` only, OR add `redirect_upload_content_type_middleware` in allowed module `blog_handlers_middleware` (`middleware/serde.cppm:90-126`) accepting `multipart/form-data`; current `type.starts_with("application/json")` (`:98`) returns 415 for Ghost's multipart (`routes.js:509`).
4. Output — REPLACE. `redirects_upload_post/handler.cppm:23-27`: replace echoed `body` (from `mutations.create("redirects",body).body()`) with fixed acknowledgement `"{}"` — Ghost `customRedirects.api.replace()` returns void (`redirects.js:40`); keep `Status::OK` (200). For the check-3 400, set `Status::BAD_REQUEST` (`interfaces/io/types.cppm:37`) before `send()`.
5. DB call — REPLACE operation (replace-all, not create-one). In `redirects_upload_post/workflow/step/execute.cppm:16`: exact calls — `auto& rs = ctx.store().resources();` (`context.cppm:1101-1104`, returns `blog::api::ResourceStore&`); for each `rec` in `blog::api::parse_array(rs.list("redirects"))` (`context.cppm:595,704-715`) decode `id` with `core_serde::JsonCodec{}.decode<std::string>(it->second)` (`to_json` quotes id at `context.cppm:768-769`) then `rs.destroy("redirects", id)` (`ResourceStore::destroy`, `:665-672`); then `rs.create("redirects", record_json)` per parsed redirect (`ResourceStore::create`, `:640-651`). Drop `mutations.create("redirects", ...)` entirely. Same context-local `resource:redirects:<id>` keys (`context.cppm:725-727`); cross-request state under existing `resources` namespace, no globals, no SQL.
6. Cache — REPLACE. Ghost `cacheInvalidate:true` (`redirects.js:35`). Add in `redirects_upload_post/handler.cppm` before `send()` (`:26-27`): `res.set_header("X-Cache-Invalidate","/*");` (`include/interfaces/io/response.cppm:590`). The check-5 destroy/create cycle erases stale `resource:redirects:*` keys (`ResourceStore::destroy` → `m_cache.erase`, `context.cppm:667`) and re-puts under tag `resources` (`:645-646`); no new namespace or TTL.
7. Auth — same gap as download check 7: `handler.cppm:18-22` bearer check → 401, no docName/method; Ghost `permissions:true` + `docName:'redirects'` (`redirects.js:12,33`) plus multipart middleware (`routes.js:509-510`). Add `require_permission(req,res,next,send)` in `blog_handlers_middleware` keyed `docName="redirects"`, action `"edit"`, wired in `RedirectsRoutes::install` upload lambda (`routes.cppm:34-40`) ahead of `handler.redirects_redirects_upload_post().handle(...)`; apply the check-3 content-type fix too.
8. Verdict + evidence — after 3,4,5,6,7 → `MATCH`. Recheck `routes.js:506-512`, `redirects.js:12,32-41`, `redirect-config-parser.ts:14-96` vs `routes.cppm:19,33-42`, `redirects_upload_post/handler.cppm:17-28`, `redirects_upload_post/workflow/step/execute.cppm:11-17`.

Ambiguity — multipart boundary parsing and 400 propagation. Short snippet for checks 3/5:

```cpp
// redirects_upload_post/workflow/step/execute.cppm — add `import core_serde;`
// `payload`, `filename` = name="redirects" part bytes and filename from in.raw_body()
std::vector<blog::api::RecordDto> parsed;
if (filename.ends_with(".yaml") || filename.ends_with(".yml")) {
    // parse 301:/302: mappings into {from,to,permanent}; values pre-quoted with JsonCodec{}.encode
} else {
    for (const auto& rec : blog::api::parse_array(payload)) {
        if (!rec.contains("from") || !rec.contains("to")) return StepResponse<Output>{"__BAD_REQUEST__"};
        parsed.push_back(rec);
    }
    if (parsed.empty()) return StepResponse<Output>{"__BAD_REQUEST__"};
}
auto& rs = ctx.store().resources();
for (const auto& rec : blog::api::parse_array(rs.list("redirects"))) {
    const auto it = rec.find("id"); if (it == rec.end()) continue;
    const auto id = core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
    rs.destroy("redirects", id);
}
for (const auto& r : parsed) rs.create("redirects", blog::api::record_to_json(r));
return StepResponse<Output>{"{}"};
```

`record_to_json` (`context.cppm:611-620`) writes raw values: store `from`/`to` pre-quoted via `core_serde::JsonCodec{}.encode(...)` and `permanent` as literal `true`/`false`. Handler maps `"__BAD_REQUEST__"` to `Status::BAD_REQUEST`.

## Central registrations needed

No shared file was edited. The following are optional maintainer follow-ups; none
block the local fixes above.

1. `include/blog/handlers/middleware/base.cppm` (+ new `permission.cppm`) —
   promote the local `redirects_routes_detail::require_permission` into
   `blog_handlers_middleware` as `require_permission(req,res,send,docName,action)`.
   The local copy mirrors `webhooks_routes_detail::require_permission`
   (`external/webhooks/routes.cppm:12-55`) and uses only already-exported
   `write_error`/`ErrorEnvelope`/`ErrorItem`, so no new globals or modules.
2. `include/blog/handlers/middleware/serde.cppm` — add a multipart-aware
   `redirect_upload_content_type_middleware` accepting `multipart/form-data`, so
   the JSON gate does not have to be scoped to `/download`. The local fix
   (`Route{"/download"}.use(content_type_middleware)`) already lets Ghost
   multipart uploads through.
3. `include/blog/handlers/context/context_context.cppm` — no change: `ResourceSchema`
   already maps `redirects` (`id,from,to,...`) and `to_json` preserves unknown
   fields, so `permanent` survives the upload → download round trip and drives 301/302.
4. `include/blog/handlers/BUILD.bazel` — no change: `blog_handlers` already deps on
   `//include/core/serde:core_serde` (line 902).
5. `include/blog/handlers/routes.cppm` / `all.cppm` — no change: `RedirectsRoutes::install`
   was already registered (`routes.cppm:38`).

## Missing in C++

None. Both Ghost routes (`GET /redirects/download`, `POST /redirects/upload`) have C++ counterparts.

#### Fix

No change required. Both Ghost routes have C++ counterparts; `MISSING` count stays 0. Fix blocks above close checks 1–8 for each route.

## Extra in C++

None. `include/blog/handlers/system/redirects/routes.cppm` installs exactly two routes — GET `/download` (`:21-31`) and POST `/upload` (`:32-42`) — matching the two Ghost controller routes. Grep for `redirects` under `include/blog/handlers/**` (`routes.cppm`, `all.cppm:198-199,544-545,793-794`, `BUILD.bazel:578-583,818`) found no additional redirect route registrations or handlers.

#### Fix

No change required. `EXTRA` count stays 0; no C++-only route to remove or map.
