# omni

C++20 server (Bazel) with a Rust client SDK and a copied Ghost frontend.

## Layout

- `include/`, `plugins/` — C++ server and plugins. Business logic that already exists here stays.
- `sdk/` — Rust client SDK. All **new** business logic and every server request lives here.
- `visual/` — frontend copied from the Ghost monorepo (`apps/`, `koenig/`, `packages/`,
  `contracts/`). UI only; no request construction or domain rules.
- `plugins/generator/openapi_generator/` — restored OpenAPI generator plugin plus
  `openapi_dump`, which writes `sdk/openapi/openapi.json` from registered routes.

## Request flow

```
visual (TypeScript) -> wasm-bindgen glue -> sdk/bindings (Rust) -> sdk/client
                    -> sdk/api (progenitor, generated from openapi.json) -> C++ server
```

## Common commands

```sh
make openapi       # regenerate sdk/openapi/openapi.json from C++ route metadata
make sdk-build     # build all Rust crates for the host
make sdk-wasm      # build wasm32 cdylib + JS glue into visual/packages/congelado-sdk/pkg
make sdk-lint      # rustfmt check (Google Rust style)
make visual-install && make visual-dev
```
