# Bazel-backed developer entry points.
#
# Override the target pattern with `make build TARGETS=//include/...` or the
# Bazel binary with `make BAZEL=bazelisk`. Configs come from .bazelrc.

BAZEL   ?= bazel
TARGET  ?= //include/app/testing:app_api_server
TARGETS ?= //...
FMT     ?= clang-format
FMT_DIRS = include bazel
LINT_DIRS = $(FMT_DIRS)

.PHONY: all build dev test prod release clean run query deps status fmt format lint help core fresh openapi sdk-build sdk-wasm sdk-lint visual-install visual-dev visual-e2e

all: build

## build: build every target (default config)
build:
	$(BAZEL) build $(TARGETS)

## dev: build with debug symbols and no stripping (--config=debug)
dev:
	$(BAZEL) build --config=debug $(TARGETS)

## test: run all Bazel test targets
test:
	$(BAZEL) test $(TARGETS)

## prod / release: optimized build (--config=prod)
prod release:
	$(BAZEL) build --config=prod $(TARGETS)

## run: run a single binary target, e.g. make run TARGET=//plugins/generator/openapi_generator:openapi_dump
run:
	$(BAZEL) run $(TARGET)

## core: build just the new core module libraries (no third-party repos needed)
core:
	$(BAZEL) build //include/core/database:core_database //include/core/cache:core_cache

## query: list all targets; override with PATTERN
query:
	$(BAZEL) query '$(if $(PATTERN),$(PATTERN),"$(TARGETS)")'

## deps: show the dependency graph for a target, e.g. make deps TARGET=//include/core/cache:core_cache
deps:
	$(BAZEL) query 'deps($(TARGET))' --output=tree

## status: Bazel workspace info
status:
	$(BAZEL) info

## clean / fresh: drop the Bazel output tree
clean fresh:
	$(BAZEL) clean

## fmt / format: format C++ sources and Starlark files in place
fmt format:
	@find $(FMT_DIRS) -type f \( -name '*.cppm' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' \) \
		-not -path '*/build/*' -print0 | xargs -0 -r $(FMT) -i

## lint: verify formatting without modifying files
lint:
	@find $(LINT_DIRS) -type f \( -name '*.cppm' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' \) \
		-not -path '*/build/*' -print0 | xargs -0 -r $(FMT) --dry-run -Werror

## openapi: regenerate sdk/openapi/openapi.json from registered C++ routes
openapi:
	$(BAZEL) build //plugins/generator/openapi_generator:openapi_dump
	./bazel-bin/plugins/generator/openapi_generator/openapi_dump sdk/openapi/openapi.json

## sdk-build: build all Rust SDK crates (native host)
sdk-build:
	$(BAZEL) build //sdk/...

## sdk-wasm: build the Rust SDK for wasm32 and generate the visual/ JS glue
sdk-wasm:
	$(BAZEL) build //sdk/bindings:sdk_wasm_cdylib --platforms=@rules_rust//rust/platform:wasm32
	wasm-bindgen --target web --out-dir visual/packages/congelado-sdk/pkg bazel-bin/sdk/bindings/sdk_bindings.wasm

## sdk-lint: verify hand-written Rust formatting against the Google-style config
sdk-lint:
	@find sdk -name '*.rs' -print0 | xargs -0 -r env RUSTC_BOOTSTRAP=1 rustfmt --check --edition 2021 --config-path sdk/rustfmt.toml

## visual-install / visual-dev: Ghost frontend workspace copied into visual/
visual-install:
	cd visual && corepack pnpm install

visual-dev:
	cd visual && corepack pnpm dev

## visual-e2e: Playwright browser tests for the SDK/theme integration
visual-e2e:
	cd visual && corepack pnpm exec playwright test --config playwright.e2e.config.ts

## help: show this list
help:
	@grep -E '^## ' $(MAKEFILE_LIST) | sed 's/^## //'
