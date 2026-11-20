# Parity: poll

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/poll.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2470-2480` (scope `/poll`, `GET /status/{poll_id}`, gated `#[cfg(all(feature = "oltp", feature = "v1"))]`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/poll.rs` (`retrieve_poll_status`) + DTO `docs/hyperswitch/crates/api_models/src/poll.rs`, input `docs/hyperswitch/crates/router/src/types/api/poll.rs`
- C++: `include/app/handlers/poll.cppm`
- Checked: 2026-11-20 (Lane G re-verification)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Single route `GET /poll/status/{poll_id}`; upstream `app.rs:2475-2478` vs C++ scope `/poll` + `/status/:poll_id` (`poll.cppm:162-179`). Feature gating (`oltp`+`v1`) still not represented — see disposition. |
| Input types | PARTIAL | Upstream typed `PollId { poll_id: String }` from `web::Path<String>` (`routes/poll.rs:31-36`, `types/api/poll.rs:1-4`); C++ takes trailing `:poll_id` segment via `RequestPath` with an empty-id guard, no body decode and no wrapper DTO (`poll.cppm:87-106`). |
| Output types | PARTIAL | Both return `PollResponse { poll_id, status }` with three statuses. Upstream `snake_case` (`api_models/poll.rs:13-20`); C++ now emits `pending`/`completed`/`not_found` through a local `PollWireResponse`/`PollStatusWireName` substitute (`poll.cppm:29-39`, `:145-148`, `:211-226`, `:260-271`). Kept PARTIAL: wire shape comes from a documented local DTO, not `dto/poll.cppm:12-17`. |
| Auth/permissions | PARTIAL | Upstream dynamic `SdkAuthorizationAuth` if `Authorization` header present, else `PublishableKeyAuth`, both `allow_connected_scope_operation=true` (`routes/poll.rs:37-50`). C++ now selects JWT vs PUBLISHABLE_KEY from the auth rule registry with `allow_connected(true)`/`allow_platform(false)` via `authorizer()` (`poll.cppm:55-80`). Kept PARTIAL: local scheme-based rules substitute for upstream auth types. |
| Persistence | PARTIAL | Upstream Redis key `poll_{processor_merchant_id}_{poll_id}`, miss → `PollNotFound` (`core/poll.rs:23-38`). C++ now prefixes key with requesting merchant id from header/auth context and returns NOT_FOUND/`PollNotFound` on miss (`poll.cppm:108-129`). Kept PARTIAL: merchant identity substitutes processor account id; no Redis connection-error channel. |
| Domain logic | PARTIAL | Was STUB. Now strict status parse with `InternalServerError` (`poll.cppm:131-143`, `:189-208`), `PollNotFound` (`:120-129`), flow metric (`:85`), snake_case wire (`:145-148`, `:211-226`), dynamic auth (`:55-80`). Remain documented substitutes/external STUBs: no processor domain object, Redis connection-error mapping, typed event, feature gating, payment-side writer. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/poll/status/{poll_id}` | `retrieve_poll_status` (`Flow::RetrievePollStatus`) | `handle_retrieve_poll_status` (`poll.cppm:44-156`) | Up: `PollId { poll_id: String }` from path (`routes/poll.rs:31-36`); C++: trailing `:poll_id` segment with empty-id guard, no body decode (`poll.cppm:87-106`) | Up: `ApiModels::PollResponse` JSON (`core/poll.rs:43-47`, `api_models/poll.rs:5-11`); C++: `PollWireResponse` JSON with snake_case status (`poll.cppm:29-39`, `:145-155`, `:260-271`) | Up: dynamic `SdkAuthorizationAuth`/`PublishableKeyAuth`, connected-scope allowed (`routes/poll.rs:37-50`); C++: dynamic JWT/PUBLISHABLE_KEY via auth rule registry + `authorizer()`, allow_connected(true)/allow_platform(false) (`poll.cppm:55-80`) | Up: Redis via `store.get_redis_conn()`, key `poll_{proc_merchant_id}_{poll_id}`, `PollNotFound` on miss (`core/poll.rs:17-38`); C++: `cache().get` key `poll_{merchant_id}_{poll_id}`, NOT_FOUND/`PollNotFound` on miss, no Redis error channel (`poll.cppm:108-129`) | PARTIAL | Up `routes/poll.rs:12-63`, `app.rs:2473-2480`, `core/poll.rs:11-48`; C++ `poll.cppm:44-156`, `:189-226`, `install` `:158-179` |

## Not ported
- Dynamic auth selection: upstream picks `SdkAuthorizationAuth` when `Authorization` header present, otherwise `PublishableKeyAuth` via `HeaderAuth`; both set `allow_connected_scope_operation=true`, `allow_platform_self_operation=false` (`routes/poll.rs:37-50`). C++ always checks `AppPermission::PAYMENT_READ` (`poll.cppm:28-35`).
- `api::server_wrap` wrapper with `Flow::RetrievePollStatus`, `LockAction::NotApplicable`, and `auth.platform.get_processor()` extraction (`routes/poll.rs:33`, `:51-61`). C++ bypasses the framework entirely.
- Processor-scoped Redis key: `super::utils::get_poll_id(processor.get_account().get_id(), request_poll_id)` prefixes `poll_{processor_merchant_id}_` to restrict access since poll id is a free-form request string (`core/poll.rs:23-25`, `utils.rs:2452-2457`). C++ key is only `poll_{poll_id}` with a fixed prefix (`poll.cppm:61`).
- `PollNotFound` on cache miss: upstream returns 404 `ApiErrorResponse::PollNotFound { id }` (`core/poll.rs:36-38`). C++ seeds `pending` and returns 200 (`poll.cppm:63-66`).
- `parse_enum("PollStatus")` with `InternalServerError` on invalid value (`core/poll.rs:39-42`). C++ uses ad-hoc case-insensitive string comparisons with a silent `Pending` default (`poll.cppm:68-74`).
- Redis connection error mapping (`Failed to get redis connection`, per-key fetch context) and `change_context` chains (`core/poll.rs:17-21`, `:26-35`). C++ has no such error paths.
- `PollStatus` wire format: upstream `strum`/`serde` `serialize_all = "snake_case"` → `pending`/`completed`/`not_found` (`api_models/poll.rs:13-20`). C++ relies on rfl enum-name serialization (`Pending`/`Completed`/`NotFound`) with no snake_case customization (`dto/poll.cppm:12-17`, handler `:69-74`) — response `status` value will differ.
- `ApiEventMetric for PollResponse` emitting `ApiEventsType::Poll { poll_id }` (`api_models/poll.rs:22-28`). Absent in C++.
- `#[cfg(all(feature = "oltp", feature = "v1"))]` route gating (`app.rs:2472`). C++ registers the route unconditionally (`poll.cppm:85-101`).
- Utoipa OpenAPI annotation / `security(("publishable_key" = []))` and 404 response documentation (`routes/poll.rs:13-26`). C++ only sets a route `.summary(...)` (`poll.cppm:99`).
- Upstream payment-side writer that seeds the Redis key with `pending`/`completed` status; not present in C++ (acknowledged in `poll.cppm:54-60`), so the read path is effectively isolated.

## Implementation disposition

Re-verified against the current handler; each original `## Not ported` bullet is closed below. Verdict key: IMPLEMENTED = behavior present without a documented substitute; SUBSTITUTED = local deterministic substitute, wire/behavior preserved; DEFERRED = justified external-dependency STUB (upstream file:line, missing dependency, local behavior, removal condition).

- Dynamic auth selection — SUBSTITUTED — auth rule registry + header-driven scheme (JWT when `Authorization` present, else PUBLISHABLE_KEY), `allow_connected(true)`/`allow_platform(false)`, resolved by `authorizer()` (`poll.cppm:55-80`); no upstream `SdkAuthorizationAuth`/`HeaderAuth` types locally.
- `api::server_wrap` wrapper — SUBSTITUTED — flow recorded via `metrics().record_flow("RetrievePollStatus")` (`poll.cppm:82-85`); `LockAction::NotApplicable` needs no lock, and `auth.platform.get_processor()` is replaced by merchant id (`poll.cppm:112-115`); `app_metrics` has no Flow/LockAction object.
- Processor-scoped Redis key — SUBSTITUTED — requesting merchant id from header, else auth context, prefixes `poll_{id}_` (`poll.cppm:108-116`); no `domain::Processor` / account id exists in this port.
- `PollNotFound` on cache miss — IMPLEMENTED — `NOT_FOUND` error with `kind_name("PollNotFound")` and poll id in message (`poll.cppm:120-129`).
- `parse_enum("PollStatus")` — IMPLEMENTED — strict `PollStatusParser` returns `nullopt` on unknown value → `INTERNAL`/`InternalServerError` (`poll.cppm:131-143`, `:189-208`); accepts snake_case plus PascalCase for local cache writers.
- Redis connection error mapping — DEFERRED — justified STUB: upstream `core/poll.rs:17-21`, `:26-35` maps `get_redis_conn`/per-key fetch failures with `change_context`; local `Cache` exposes hit/miss only and no connection-error channel (`poll.cppm:118-119`), miss maps to `PollNotFound`. Remove by supplying a Redis-backed cache service with error propagation.
- `PollStatus` wire format — IMPLEMENTED — `PollWireResponse` + `PollStatusWireName` serialize `pending`/`completed`/`not_found` snake_case (`poll.cppm:29-39`, `:145-148`, `:211-226`, `:260-271`).
- `ApiEventMetric for PollResponse` — SUBSTITUTED — poll-id-scoped counter `poll_response.{poll_id}` carries the `ApiEventsType::Poll { poll_id }` dimension (`poll.cppm:151-153`); `app_metrics` has no typed event object.
- Feature gating (`oltp`+`v1`) — DEFERRED — upstream `app.rs:2472` gates the route at compile time; the port has no compile-time feature-flag/config gating mechanism and registers unconditionally (`poll.cppm:158-179`). Remove by adding a route-registration feature gate.
- Utoipa OpenAPI annotation — SUBSTITUTED — typed `.summary()`/`.description()` carry auth and 404 semantics as text (`poll.cppm:172-178`); `utils::openapi::ApiRoute` has no structured `security(...)` scheme or response-schema objects.
- Payment-side poll writer — DEFERRED — external dependency on the payment domain to seed `poll_{merchant}_{id}` with `pending`/`completed` (read contract `core/poll.rs:23-25`); this handler only reads (`poll.cppm:120`) and no writer exists in the port. Remove by implementing the writer in the payment-status flow.
