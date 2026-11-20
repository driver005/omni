# omni

C++ server (Bazel, GCC 16 + C++26 modules) with a Rust client SDK and a copied Ghost frontend.

> **TIP — GCC 16 module header bug:** On a fresh Linux install you will very likely hit
> `error: no such file or directory: '.../include/c++/16/bits/std.cc'` on the first build.
> This is upstream GCC bug [libstdc++/119266](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=119266)
> in `contrib/relpath.sh`: the generated `libstdc++.modules.json` resolves `bits/std.cc` one
> directory too shallow, because headers install under `/usr/include/c++/<ver>/` while the
> manifest lands under `/usr/lib/gcc/<target>/<ver>/`. It is not a build-system bug.
> Run `scripts/fix-gcc16-modules.sh` once after installing GCC 16 (and again after a GCC
> major-version bump) to add the symlink that makes the manifest's path resolve correctly,
> without modifying any package-managed file.

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
scripts/fix-gcc16-modules.sh  # one-time GCC 16 libstdc++ module symlink fix
make openapi       # regenerate sdk/openapi/openapi.json from C++ route metadata
make sdk-build     # build all Rust crates for the host
make sdk-wasm      # build wasm32 cdylib + JS glue into visual/packages/congelado-sdk/pkg
make sdk-lint      # rustfmt check (Google Rust style)
make visual-install && make visual-dev
```
