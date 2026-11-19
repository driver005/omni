# Rust SDK

All server requests and new business logic live here. The frontend in `../visual` consumes
this crate through the WASM binding in `bindings/`.

## Crates

- `api/` — `progenitor::generate_api!` expands `openapi/openapi.json` into a typed `Client`.
  Generated code is macro output and is not hand-edited.
- `client/` — `ApiClient` owns the generated client, base URL, auth header and timeouts.
- `auth/` — session/token state.
- `domain/` — hand-written business rules (no generated types here).
- `error/` — shared error enum.
- `bindings/` — `wasm-bindgen` facade exposed to TypeScript.

## Regenerating API calls

```sh
make openapi     # C++ routes -> sdk/openapi/openapi.json
make sdk-build   # progenitor expands the spec at compile time
```

Nothing in `api/` is hand-written per endpoint. Adding a route in `include/blog/api` and
re-running `make openapi` is what changes the generated client.

## Style

Hand-written Rust follows the
[Google Rust Style Guide](https://google.github.io/styleguide/rustguide.html), enforced by
`rustfmt.toml` (`max_width = 100`, `imports_granularity = "Module"`) and `make sdk-lint`.
Generated macro output is treated as generated code.
