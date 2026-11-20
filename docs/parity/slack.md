# Parity: slack

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/slack.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:407-408`
- C++: `include/blog/handlers/external/slack/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass applied: 2026-11-20 (folder-only edits; no shared file edited)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Single route `POST /slack/test` flipped MISMATCH → MATCH. MATCH uses the same rubric as
the other strict-pass reports: primary observable contract (verb/path, input, status,
output, permission gate, side effect at the allowed primitive level) agrees; residual
gaps with no allowed C++ primitive are listed below and do not change the verdict.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/slack/test` | `sendTest` (`events.emit('slack.test')`) | POST `/slack` + `/test` | `slack_slack_test_post().handle` | none on both sides | Ghost: `{}` (`res.json(result||{})`); C++: 200 `{}` from step `Output` map | Ghost: no model/service write (event only); C++: no store write — `blog::handlers::outbox::global_outbox().emit(Event{"slack.test",...})` | Ghost: `cacheInvalidate:false`, app-level `Cache-Control`; C++: `Cache-Control` set, no `X-Cache-Invalidate`, no store/cache write | Ghost: `mw.authAdminApi` + `permissions:false`; C++: `require_ghost_permission("slack","add")` via `core_auth:ghost`, no content-type gate | MATCH | Ghost `routes.js:408`, `slack.js:6-14`, `admin/app.js:24,33-34`, `admin/middleware.js:110-118`; C++ `routes.cppm:19-36`, `handler.cppm:32-68`, `step/execute.cppm:25-33`, `outbox_shim.cppm:65-165`, `middleware/ghost_permission.cppm:63-151` |

## Route details

### POST `/slack/test` — MATCH

- Route identity: agrees. Ghost `router.post('/slack/test', mw.authAdminApi, http(api.slack.sendTest))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:408`); C++ `Router{router, "/slack"}` + `Route{"/test"}.post(...)` (`include/blog/handlers/external/slack/routes.cppm:25-34`). Verb + path preserved by the strict pass.
- Operation mapping: agrees at allowed primitive level. Ghost `sendTest` emits `events.emit('slack.test')` (`slack.js:12-14`), consumed by `SlackPingService.handleTestEvent()` (`docs/Ghost/ghost/core/server/services/slack-ping/slack-ping-service.ts:284-288`, listener at `:302`). C++ workflow `slack_slack_test_post` (`slack_test_post/workflow/flow/main.cppm:9`) runs `ExecuteStep::invoke`, which emits `blog::handlers::outbox::Event{"slack.test", ...}` through the registered `blog_handlers:outbox_shim` partition (`slack_test_post/workflow/step/execute.cppm:25-29`). No `slack` record or any store write remains.
- Input parity: both sides consume nothing. Ghost declares no `options[]`, `data[]`, `validation{}` (`slack.js:7-15`). C++ step uses `Input=interfaces::io::IRequest&` but ignores it (`step/execute.cppm:12,16-17`); no body read. The former router `content_type_middleware` (415/406 on a no-input route) was removed (`routes.cppm:19-24`), matching Ghost's permissive global body parsers (`admin/app.js:33-34`).
- Output parity: Ghost output serializer sets `frame.response = undefined` → `res.json(result||{})` → `{}`. C++ step returns `Output=std::map<std::string,std::string>{{"status","200"},{"body","{}"}}` (`step/execute.cppm:33`) and the handler writes status 200 with raw bytes `{}` (`handler.cppm:52-68`). No envelope, no `{"slack":[...]}` body, no empty body.
- DB call / side effects: Ghost performs no model/service write. C++ performs no `ctx.store()` call at all; the only side effect is the `slack.test` event on the process-wide outbox (`outbox_shim.cppm:65-141`, `global_outbox()` at `:158-163`). No `resource:outbox:<id>` cache entry, fabricated id, or `"1970-01-01"` timestamps remain.
- Cache parity: Ghost sets `headers.cacheInvalidate: false` (`slack.js:8-10`) → no `X-Cache-Invalidate`, plus app-level `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` (`admin/app.js:24`). C++ writes the same `Cache-Control` string (`handler.cppm:61-65`) and no `X-Cache-Invalidate` (falsy `cacheInvalidate`). No cache service on either side.
- Auth / permissions: Ghost uses `mw.authAdminApi` with controller `permissions:false`, `docName:'slack'` (`admin/routes.js:408`, `slack.js:6,11`). C++ handler defaults `x-blog-context: ADMIN_API` and (`x-blog-role: Administrator`, `x-blog-permissions: *`) only when `Authorization` is present, then calls `blog::handlers::middleware::require_ghost_permission(req, res, next, send, "slack", "add")` (`handler.cppm:32-46`). Denials produce Ghost-shaped 401/403 envelopes via `write_error` (`middleware/ghost_permission.cppm:96-151`, backed by `core_auth:ghost::GhostAuthorizer`). No bare token-presence check or raw `{"error":"Unauthorized"}` body remains.

#### Fix (applied 2026-11-20)

The original `#### Fix` checks were implemented as follows; exact symbols and replacements:

- **Check 1 (route identity) — closed.** `blog::api::SlackRoutes::install` (`routes.cppm:11-37`) keeps `Router{router, "/slack"}` + `Route{"/test"}.post(...)`.
- **Check 2 (operation mapping) — applied.** Replaced `ctx.store().resources().create("outbox", ...)` in `blog::handlers::slack::slack_test_post::workflow::ExecuteStep::invoke` (`step/execute.cppm:16-34`) with a shared outbox event emit:
  ```cpp
  blog::handlers::outbox::global_outbox().emit(
      blog::handlers::outbox::Event{
          "slack.test",
          R"({"text":"Heya! This is a test notification from your Ghost blog :smile:. Seems to work fine!","unfurl_links":true})",
          ""});
  ```
  Import `import :outbox_shim;` (already registered in `include/blog/handlers/BUILD.bazel:919` and exported by `all.cppm:13`) — no new partition, no new global state.
- **Check 3 (input parity) — applied.** Step ignores `Input`; no body read; router content-type middleware removed (`routes.cppm:19-24`).
- **Check 4 (output parity) — applied via string/map Output with `s_ctx`.** `using Output = std::map<std::string,std::string>` (`step/execute.cppm:13`) returned `{{"status","200"},{"body","{}"}}` (`:33`); handler reads both keys (`handler.cppm:52-59`) and writes status 200 / body `{}` (`:66-68`). Static `blog::BlogContext* s_ctx` remains set in the handler ctor (`handler.cppm:16`, `step/execute.cppm:40,43`).
- **Check 5 (DB call parity) — applied.** Removed the `ResourceStore::create("outbox", ...)` call entirely; no store write, no cache entry.
- **Check 6 (cache parity) — applied.** `Cache-Control` added (`handler.cppm:61-65`); no `X-Cache-Invalidate` header.
- **Check 7 (auth) — applied.** Bare token check replaced with `require_ghost_permission("slack","add")` (`handler.cppm:32-46`); router content-type/require-auth middleware dropped.
- **Check 8 (verdict) — flipped to MATCH.** Path+verb, admin-auth gate, no input, 200 `{}`, `slack.test` event, no store write, cache headers all agree at the allowed primitive level.

## Missing in C++

None. The single Ghost route `POST /slack/test` has a C++ counterpart. `MISSING` stays 0.

## Extra in C++

None. `SlackRoutes::install` registers exactly one `.post(...)` route (`/slack` + `/test`), the counterpart of Ghost's `POST /slack/test`. `EXTRA` stays 0.

## Residual gaps and new-partition needs

Documented, not fixed here (no shared file edited, no new partition created):

1. **Slack webhook delivery** — Ghost's listener performs a real HTTP POST to `slack_url` (`slack-ping-service.ts:242-248`). C++ emits the `slack.test` event but has no listener/adapter that dispatches it. Needs a **new Slack adapter partition** in `include/core/adapters/` (alongside Stripe/Tinybird/SMTP/Webmention) wired via `core_outbox::register_adapter`; settings resolution (`slack_url`, `slack_username`, `title`) and `blogIcon.getIconUrl({absolute:true})` would belong in that adapter.
2. **Async fire-and-forget + failure logging** — no equivalent; the C++ outbox emit is synchronous queueing. Ghost also does not surface delivery failure to the route caller.
3. **Integration-token 403** — `core_auth:ghost::Principal` has no representation for Ghost's integration-token allowlist (`admin/middleware.js:54-100`); the shared middleware cannot reproduce that specific 403 shape.
4. **Framework `etag` / `content-version`** — no shared primitive computes Ghost's content-version; C++ writes only app-level `Cache-Control`.

## Verification note

Strict pass touched only `include/blog/handlers/external/slack/` and the two parity docs. Shared files (`all.cppm`, central `routes.cppm`, `BUILD.bazel`, middleware, context) were not edited. The pre-existing Bazel dependency cycle in `//include/blog/handlers:blog_handlers` and unrelated compile errors in other groups' folders (`discovery/recommendations/browse`, `auth/session/create`) remain untouched, so a full shared-target build is still blocked; parity is argued from the registered shared-abstraction contracts in `docs/parity/behavior/CONTRACTS.md` and existing strict-pass usage patterns (`admin/labels`).
