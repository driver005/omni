# Bazel-backed developer entry points.
#
# Override the target pattern with `make build TARGETS=//include/...` or the
# Bazel binary with `make BAZEL=bazelisk`. Configs come from .bazelrc.

BAZEL   ?= bazel
TARGETS ?= //...
FMT     ?= clang-format
FMT_DIRS = include bazel third_party

.PHONY: all build dev test prod release clean run query deps status fmt format lint help core fresh

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

## run: run a single target, e.g. make run TARGET=//include/src:server
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
	@find $(FMT_DIRS) -type f \( -name '*.cppm' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' \) \
		-not -path '*/build/*' -print0 | xargs -0 -r $(FMT) --dry-run -Werror

## help: show this list
help:
	@grep -E '^## ' $(MAKEFILE_LIST) | sed 's/^## //'
