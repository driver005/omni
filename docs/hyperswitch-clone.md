# Hyperswitch upstream clone

- Repo: https://github.com/juspay/hyperswitch
- Clone command: `git clone --depth 1 https://github.com/juspay/hyperswitch.git docs/hyperswitch`
- Ref: `main` HEAD at clone time
- SHA: `7736d55adaf235f7af74e133ae408573b1826823`
- Commit date: 2026-11-19 08:12:45 +0000
- Commit subject: `fix(ucs): map AliPay HK redirect wallet in payment requests (#15308)`
- License: Apache-2.0 (`docs/hyperswitch/LICENSE`)

This clone is the frozen read-only source for the C++ conversion into `include/app/`.
Do not edit files under `docs/hyperswitch/`. `docs/hyperswitch` is listed in `.bazelignore`
so upstream BUILD files do not join this workspace's `//...`.

## Conversion sources

- Types: `docs/hyperswitch/crates/api_models/src/*.rs` — 61 files, 1,681,148 bytes
- Routes: `docs/hyperswitch/crates/router/src/routes/*.rs` — 60 files, 902,260 bytes

## Mapping policy

- `Option<T>` -> `std::optional<T>`; `Vec<T>` -> `std::vector<T>`; maps -> `std::unordered_map`; Rust enums -> `enum class`.
- Cross-crate types mapped onto existing `core` abstractions where equivalent; otherwise translated into `app` value types.
- No crypto/PCI logic. Card numbers, secrets, signatures are opaque value types.
- No builds until the whole conversion is done.

## Conversion status (complete)

- 61 `api_models` source files -> 60 type partitions: 15 in `include/app/model`, 45 in `include/app/dto` (`lib.rs` used for inventory only).
- 60 `router/src/routes` source files -> 60 handler partitions in `include/app/handlers`.
- Shared wiring: `include/app/handlers/all.cppm` (`AppHandlers`), `routes.cppm` (`AppRoutes`), `serde_setup.cppm` (JSON serde format).
- Single build gate after all conversions: `bazel build //...` exit 0; `bazel test //...` 8/8 pass.
- Upstream `docs/hyperswitch` is read-only; `git -C docs/hyperswitch status --short` is empty.

## Domain logic architecture

- Each `include/app/handlers/<domain>.cppm` owns one domain class holding **all** operations for that domain (e.g. `ApiKeysRouteHandlers::create/retrieve/update/revoke/list`). No per-method logic classes.
- Domain classes are constructed once at startup with `app::context::AppContext`; methods still take `(IRequest&, IResponse&, send)`.
- `include/app/context/` provides the core-only service facade: `core_serde` formats, `core_database`, `core_cache`, `core_workflow`. Plugin backends are injected by `AppHandlers`, never imported by handlers.
- Handler logic uses only core crates — `serde::Ser::decode_generic`/`encode_json` for wire shape and `core::cache::Cache` via `AppContext` for per-domain state. No `core_operation`, no plugin imports, no free/static functions.
- Error paths use `core_error`: `AppErrorKind` enum + `AppError`/`AppErrorBuilder` with `serde::Serializable`, emitting `{"error":"INVALID_JSON","message":"..."}`. No raw error JSON strings remain.
- 436 handlers across 57 domain classes are implemented this way.
- Scoped build passes. Full `bazel test //...` is currently blocked by an unrelated external blog refactor.
