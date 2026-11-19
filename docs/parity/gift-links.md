# Parity: gift-links

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/gift-links.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 71-78)
- C++: `include/blog/handlers/admin/posts/routes.cppm`, `include/blog/handlers/admin/pages/routes.cppm`, `include/blog/handlers/membership/gift_links/routes.cppm` (installed via `include/blog/handlers/routes.cppm:19,92`; handlers registered in `include/blog/handlers/all.cppm:47-50,250-252,342-354,393-396,603-605,642-645`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 7 |
| MISSING | 0 |
| EXTRA | 0 |

All 7 Ghost routes have a C++ route with matching path + verb (3 posts, 3 pages, 1 remove_all). Every C++ counterpart is a placeholder: it parses no `id`, performs no real service/DB operation, and returns hardcoded/serialized stub data with a different envelope than Ghost. No routes are missing or extra.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/posts/:id/gift_links` | `browse` | `admin/posts/routes.cppm:83-91` (`/posts` → `/:id` → `/gift_links`, GET) | `posts_gift_links_browse()` → `PostsGiftLinksBrowseHandler` | Ghost: option `id` required (`gift-links.ts:38-39`); C++: no parsing, `Input=IRequest&` only (`posts/gift_links_browse/workflow/step/execute.cppm:13-21`) | Ghost: `Post {id, giftLinks[]}` from `getPost` (`service.ts:24-47`); C++: hardcoded `{"gift_links":[{"id":"1","url":"https://ghost.local/link"}]}`, 200 (`execute.cppm:21`; `handler.cppm:32-33`) | Ghost: knex JOIN `posts`/`post_gift_links`/`gift_links` (`service.ts:30-35`); C++: `store()` acquired but unused (`execute.cppm:20`) | Ghost: `cacheInvalidate:false` header (`gift-links.ts:31,37`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:72`) + `manage.gift_link(id)` & `edit.post(id)` (`gift-links.ts:14-18`); C++: only `content_type_middleware` (`posts/routes.cppm:20`), no token check | MISMATCH | Ghost `routes.js:72`, `gift-links.ts:36-46`; C++ `posts/routes.cppm:83-91`, `posts/gift_links_browse/handler.cppm:22-35`, `posts/gift_links_browse/workflow/step/execute.cppm:13-22` |
| PUT `/posts/:id/gift_links` | `ensure` | `admin/posts/routes.cppm:92-99` (`/posts` → `/:id` → `/gift_links`, PUT) | `posts_gift_links_edit()` → `PostsGiftLinksEditHandler` | Ghost: `id` required + actor context (`gift-links.ts:51-52,20-29`); C++: no `id`, no actor, no body DTO (`posts/gift_links_edit/workflow/step/execute.cppm:13-21`) | Ghost: `Post {id, giftLinks[]}` from transactional `ensure`, 200 (`service.ts:64-74`; `gift-links.ts:50`); C++: hardcoded `gift_links` stub, 200 (`execute.cppm:21`; `handler.cppm:32-33`) | Ghost: `ensureTransaction`, locked post, mint + `recordAction` (`service.ts:64-74,113-133`); C++: `store()` unused (`execute.cppm:20`) | Ghost: `cacheInvalidate:false` (`gift-links.ts:31,49`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:73`) + `manage.gift_link`/`edit.post` (`gift-links.ts:14-18`); C++: `content_type_middleware` only (`posts/routes.cppm:20`) | MISMATCH | Ghost `routes.js:73`, `gift-links.ts:48-59`; C++ `posts/routes.cppm:92-99`, `posts/gift_links_edit/handler.cppm:22-35`, `posts/gift_links_edit/workflow/step/execute.cppm:13-22` |
| POST `/posts/:id/gift_links` | `create` | `admin/posts/routes.cppm:100-107` (`/posts` → `/:id` → `/gift_links`, POST) | `posts_gift_links_create()` → `PostsGiftLinksCreateHandler` | Ghost: `id` required + actor (`gift-links.ts:64-65,20-29`); C++: no `id`, no actor, no body DTO (`posts/gift_links_create/workflow/step/execute.cppm:13-21`) | Ghost: fresh `Post {id, giftLinks[]}`, 200 (`service.ts:80-87`; `gift-links.ts:63`); C++: hardcoded `gift_links` stub, 200 (`execute.cppm:21`; `handler.cppm:32-33`) | Ghost: `ensureTransaction` + mint/reset + `recordAction('reset')` (`service.ts:80-87`); C++: `store()` unused (`execute.cppm:20`) | Ghost: `cacheInvalidate:false` (`gift-links.ts:31,62`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:74`) + `manage.gift_link`/`edit.post` (`gift-links.ts:14-18`); C++: `content_type_middleware` only (`posts/routes.cppm:20`) | MISMATCH | Ghost `routes.js:74`, `gift-links.ts:61-72`; C++ `posts/routes.cppm:100-107`, `posts/gift_links_create/handler.cppm:22-35`, `posts/gift_links_create/workflow/step/execute.cppm:13-22` |
| GET `/pages/:id/gift_links` | `browse` | `admin/pages/routes.cppm:83-91` (`/pages` → `/:id` → `/gift_links`, GET) | `pages_gift_links_get()` → `GiftLinksGetHandler` | Ghost: `id` required (`gift-links.ts:38-39`); C++: no `id` parsing (`pages/gift_links_get/workflow/step/execute.cppm:12-24`) | Ghost: `Post {id, giftLinks[]}` (`service.ts:24-47`); C++: encoded `GiftLinksResponse` with one default `LinkRecord`, 200 (`pages/gift_links_get/workflow/step/execute.cppm:17-20`; `dto/links.cppm:8-25,59`; `handler.cppm:27`) | Ghost: knex JOIN (`service.ts:30-35`); C++: `find_by_resource("pages", "1")` unreachable — `r.first=200` returns early at `execute.cppm:21` before :22-24 | Ghost: `cacheInvalidate:false` (`gift-links.ts:31,37`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:75`) + `manage.gift_link`/`edit.post` (`gift-links.ts:14-18`); C++: token check disabled by `&& !true` (`pages/gift_links_get/handler.cppm:18`), only `content_type_middleware` (`pages/routes.cppm:20`) | MISMATCH | Ghost `routes.js:75`, `gift-links.ts:36-46`; C++ `pages/routes.cppm:83-91`, `pages/gift_links_get/handler.cppm:17-27`, `pages/gift_links_get/workflow/step/execute.cppm:14-24` |
| PUT `/pages/:id/gift_links` | `ensure` | `admin/pages/routes.cppm:92-99` (`/pages` → `/:id` → `/gift_links`, PUT) | `pages_gift_links_put()` → `GiftLinksPutHandler` | Ghost: `id` required + actor (`gift-links.ts:51-52,20-29`); C++: no `id`, no actor; `body_as_string()` only in unreachable code (`pages/gift_links_put/workflow/step/execute.cppm:22`) | Ghost: `Post` from `ensure`, 200 (`service.ts:64-74`; `gift-links.ts:50`); C++: encoded `GiftLinksResponse` stub, 200 (`execute.cppm:17-20`; `handler.cppm:27`) | Ghost: transactional mint (`service.ts:64-74`); C++: `mutations.edit("pages", ...)` unreachable — early return at `execute.cppm:21` | Ghost: `cacheInvalidate:false` (`gift-links.ts:31,49`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:76`) + permissions (`gift-links.ts:14-18`); C++: token check disabled by `&& !true` (`pages/gift_links_put/handler.cppm:18`) | MISMATCH | Ghost `routes.js:76`, `gift-links.ts:48-59`; C++ `pages/routes.cppm:92-99`, `pages/gift_links_put/handler.cppm:17-27`, `pages/gift_links_put/workflow/step/execute.cppm:14-23` |
| POST `/pages/:id/gift_links` | `create` | `admin/pages/routes.cppm:100-107` (`/pages` → `/:id` → `/gift_links`, POST) | `pages_gift_links_post()` → `GiftLinksPostHandler` | Ghost: `id` required + actor (`gift-links.ts:64-65,20-29`); C++: no `id`, no actor; `body_as_string()` only in unreachable code (`pages/gift_links_post/workflow/step/execute.cppm:22`) | Ghost: fresh `Post`, 200 (`service.ts:80-87`; `gift-links.ts:63`); C++: encoded `GiftLinksResponse` stub, 200 (`execute.cppm:17-20`; `handler.cppm:27`) | Ghost: transactional mint/reset (`service.ts:80-87`); C++: `mutations.create("pages", ...)` unreachable — early return at `execute.cppm:21` | Ghost: `cacheInvalidate:false` (`gift-links.ts:31,62`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:77`) + permissions (`gift-links.ts:14-18`); C++: token check disabled by `&& !true` (`pages/gift_links_post/handler.cppm:18`) | MISMATCH | Ghost `routes.js:77`, `gift-links.ts:61-72`; C++ `pages/routes.cppm:100-107`, `pages/gift_links_post/handler.cppm:17-27`, `pages/gift_links_post/workflow/step/execute.cppm:14-23` |
| PUT `/gift_links/remove_all` | `removeAll` | `membership/gift_links/routes.cppm:19-31` (`/gift_links` → `/remove_all`, PUT) | `gift_links_gift_links_remove_all_put()` → `GiftLinksRemoveAllPutHandler` | Ghost: no options; actor from context (`gift-links.ts:74-83,20-29`); C++: no parsing; `body_as_string()` only in unreachable code (`gift_links_remove_all_put/workflow/step/execute.cppm:22`) | Ghost: `{ count }` = deleted rows, 200 (`gift-links.ts:80-83`; `service.ts:94-102`); C++: encoded `GiftLinksResponse` stub `{"gift_links":[{id,url}]}`, 200 (`execute.cppm:17-20`; `handler.cppm:27`) — no `count` | Ghost: `trx('post_gift_links').del()` + `recordAction('remove')` (`service.ts:95-101`); C++: `mutations.edit("gift_links", "", ...)` unreachable — early return at `execute.cppm:21` | Ghost: `cacheInvalidate:false` (`gift-links.ts:31,75`); C++: none | Ghost: `mw.authAdminApi` (`routes.js:78`) + `removeAll.gift_link()` (`gift-links.ts:77-79`); C++: real token check (401 if no Authorization) at `gift_links_remove_all_put/handler.cppm:18-22`, no permission check; `content_type_middleware` (`membership/gift_links/routes.cppm:20`) | MISMATCH | Ghost `routes.js:78`, `gift-links.ts:74-84`; C++ `membership/gift_links/routes.cppm:19-31`, `gift_links_remove_all_put/handler.cppm:17-27`, `gift_links_remove_all_put/workflow/step/execute.cppm:14-23` |

## Route details

### GET /posts/:id/gift_links — MISMATCH
- Ghost op `browse` requires option `id` (`gift-links.ts:38-39`), checks `manage.gift_link(id)` + `edit.post(id)` (`gift-links.ts:14-18`), and returns `getPost(id)` → `Post {id, giftLinks[]}` backed by a knex 3-table JOIN (`service.ts:24-47`). C++ step declares only `Input=IRequest&`, reads no `id`, and returns a hardcoded string (`posts/gift_links_browse/workflow/step/execute.cppm:13-21`). `store()` acquired but unused (`:20`).
- Output envelope differs: Ghost `giftLinks` (camelCase, `token`/`createdAt`) vs C++ `{"gift_links":[{"id":"1","url":"https://ghost.local/link"}]}` (`execute.cppm:21`). No `meta`/pagination on either side. Status 200 both (`gift-links.ts` browse default; `handler.cppm:32`).
- Cache: Ghost `X-Cache-Invalidate` disabled via `cacheInvalidate:false` (`gift-links.ts:31,37`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:72`) + permissions; C++ only `content_type_middleware` (`posts/routes.cppm:20`), no token or permission check in `handler.cppm:22-35`.

### PUT /posts/:id/gift_links — MISMATCH
- Ghost `ensure` (`gift-links.ts:48-59`) performs transactional mint-if-absent and records an `add` action (`service.ts:64-74`), status 200 (`:50`). C++ binds `posts_gift_links_edit()` — a different operation name (`posts/routes.cppm:97`) — returning the same hardcoded stub (`posts/gift_links_edit/workflow/step/execute.cppm:21`), no body/id/actor parsing.
- DB: Ghost locked post + mint + `recordAction` (`service.ts:64-74,113-133`); C++ `store()` unused (`execute.cppm:20`).
- Cache/auth divergence identical to GET posts row (`gift-links.ts:31,49`; `routes.js:73`; `posts/routes.cppm:20`).

### POST /posts/:id/gift_links — MISMATCH
- Ghost `create` (`gift-links.ts:61-72`) always mints a new link and records `reset` (`service.ts:80-87`), status 200 (`:63`). C++ `posts_gift_links_create()` returns the identical hardcoded stub as the browse/edit steps (`posts/gift_links_create/workflow/step/execute.cppm:21`) — no create performed.
- Input: Ghost passes `requestContextFromFrame` actor (`gift-links.ts:20-29`); C++ no actor, no `id`, no body DTO (`execute.cppm:13-21`).
- Cache/auth divergence: `gift-links.ts:31,62`, `routes.js:74`, `posts/routes.cppm:20`.

### GET /pages/:id/gift_links — MISMATCH
- Same Ghost op `browse` as posts (`gift-links.ts:36-46`, `routes.js:75`), but C++ uses separate `pages_gift_links_get()` handlers (`pages/routes.cppm:89`).
- Output: C++ serializes `GiftLinksResponse` containing one default `LinkRecord` (`pages/gift_links_get/workflow/step/execute.cppm:17-20`; DTO `dto/links.cppm:8-25,59`) → `{"gift_links":[{"id":"1","url":"https://ghost.local/link"}]}` vs Ghost `Post` object. No DB: `find_by_resource("pages", ...)` at `execute.cppm:22-24` is dead because `r.first=200` triggers early return at `:21`.
- Auth: handler has a token check but it is neutralized by `&& !true` (`pages/gift_links_get/handler.cppm:18`), so no enforcement; Ghost has full admin auth + permissions.

### PUT /pages/:id/gift_links — MISMATCH
- Ghost `ensure` (`gift-links.ts:48-59`) vs C++ `pages_gift_links_put()` (`pages/routes.cppm:97`); C++ output is the encoded `GiftLinksResponse` stub (`pages/gift_links_put/workflow/step/execute.cppm:17-20`).
- DB: `mutations.edit("pages", "{}", body)` at `execute.cppm:22` unreachable (early return `:21`); body read only there. Ghost does real transactional mint (`service.ts:64-74`).
- Auth disabled via `&& !true` (`pages/gift_links_put/handler.cppm:18`); cache none on C++ vs `cacheInvalidate:false` (`gift-links.ts:31,49`).

### POST /pages/:id/gift_links — MISMATCH
- Ghost `create` (`gift-links.ts:61-72`) vs C++ `pages_gift_links_post()` (`pages/routes.cppm:105`); stub output (`pages/gift_links_post/workflow/step/execute.cppm:17-20`).
- DB: `mutations.create("pages", ...)` unreachable (`execute.cppm:21-22`); no actor/id parsing. Auth disabled via `&& !true` (`pages/gift_links_post/handler.cppm:18`); cache none vs `cacheInvalidate:false` (`gift-links.ts:31,62`).

### PUT /gift_links/remove_all — MISMATCH
- Ghost `removeAll` returns `{ count }` from `service.removeAll` (`gift-links.ts:74-83`; `service.ts:94-102` → `trx('post_gift_links').del()`), status 200 (`:76`). C++ returns encoded `GiftLinksResponse` stub with no `count` (`gift_links_remove_all_put/workflow/step/execute.cppm:17-20`; `handler.cppm:25-27`), and `mutations.edit("gift_links", "", body)` is unreachable (`:21-22`).
- Auth: Ghost `removeAll.gift_link()` permission (`gift-links.ts:77-79`) + `mw.authAdminApi` (`routes.js:78`); C++ has an actual token check returning 401 (`gift_links_remove_all_put/handler.cppm:18-22`) but no permission/scope check. Cache none on C++ vs `cacheInvalidate:false` (`gift-links.ts:31,75`).

## Missing in C++

None. All 7 Ghost routes have a C++ route with matching path + verb:
- `/posts/:id/gift_links` GET/PUT/POST → `admin/posts/routes.cppm:83-108`
- `/pages/:id/gift_links` GET/PUT/POST → `admin/pages/routes.cppm:83-108`
- `/gift_links/remove_all` PUT → `membership/gift_links/routes.cppm:19-31`

## Extra in C++

None. Grep for `gift_links|giftLinks|remove_all` across `include/blog/handlers/**` found routes only in the three `routes.cppm` files above (plus `all.cppm`/`base.cppm`/`BUILD.bazel` registration lines); each maps 1:1 to a Ghost route in `routes.js:72-78`. No C++ gift-link route lacks a Ghost counterpart.
