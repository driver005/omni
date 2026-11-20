load("//bazel:build_defs.bzl", "congelado_cc_test", "congelado_module_library")

# CONTRACTS.md §4 — transactional outbox for events / jobs / side effects.
# Self-contained target; main agent wires it into consumers. Reuses core_events
# (ambient event bus) so drained outbox events can fan out to registered sinks.
congelado_module_library(
    name = "core_outbox",
    primary_interface = "outbox.cppm",
    visibility = ["//visibility:public"],
)

congelado_cc_test(
    name = "outbox",
    srcs = ["outbox_test.cc"],
    deps = [":core_outbox"],
    linkopts = ["-lcrypto", "-lssl"],
)
