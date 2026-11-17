# Bazel C++20 module build

Bazel is the only build system. xmake and conan are not used.

## Layout

- `bazel/build_defs.bzl` — TensorFlow-style macros (`congelado_module_library`,
  `congelado_cc_library/binary/test`, `congelado_check_deps`).
- `third_party/copts.bzl` / `conditionals.bzl` — project-wide flags.
- `bazel/system_libstdcxx.bzl` — module extension providing `@system_libstdcxx//:std`
  from the host libstdc++ `bits/std.cc` with `-std=gnu++26`.
- `MODULE.bazel` — deps:
  - BCR: `rules_cc`, `platforms`, `nlohmann_json`, `fmt`, `spdlog`,
    `googletest`, `simdjson`, `reflect-cpp`, `tomlplusplus`, `hiredis`.
  - `http_archive`: `stduuid` (GitHub v1.2.3, header-only, bundled GSL),
    `libpq` + `postgres` (Bazel-ready libpq wrapper + PostgreSQL 16.2 source).
  - Patches for the postgres archive are kept in `third_party/libpq_patches/`
    so the wrapper repo's patch files cannot shadow generated headers.

## Build

```sh
make build    # bazel build //...
make dev      # bazel build --config=debug //...
make test     # bazel test //...
make core     # core_database + core_cache
```

Verified:

```
bazel build //include/core/database:core_database \
            //include/core/cache:core_cache \
            //plugins/cache/redis_backend:redis_backend_plugin \
            //plugins/database/postgres_backend:postgres_backend_plugin
```

Both plugins produce `.a` and `.so` artifacts.

`.bazelrc` autoloads only `cc_library`/`cc_binary`/`cc_test` so older BCR BUILD
files work; `cc_toolchain_suite` is excluded (removed in Bazel 9).
