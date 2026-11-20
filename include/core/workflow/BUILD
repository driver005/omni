load("//bazel:build_defs.bzl", "congelado_cc_test", "congelado_module_library")

# core_workflow mirrors core_router / core_database / core_cron: one primary
# interface, many partitions. The driver and runtime partitions are interface
# partitions for the toolchain but their declarations are not exported, so the
# Workflow facade is the only public way to execute a workflow.

congelado_module_library(
    name = "core_workflow",
    primary_interface = "base.cppm",
    partitions = [
        # value types and contracts
        "engine.cppm",
        "value.cppm",
        "state.cppm",
        "step_options.cppm",
        "step_response.cppm",
        "step_outcome.cppm",
        "context.cppm",
        "expression.cppm",
        "step.cppm",
        "hook.cppm",
        # definition and composition
        "definition.cppm",
        "builder.cppm",
        # execution records
        "checkpoint.cppm",
        "execution.cppm",
        "registry.cppm",
        # hidden driver layer
        "driver.cppm",
        "runtime.cppm",
        # facade
        "workflow.cppm",
    ],
    deps = [],
    visibility = ["//visibility:public"],
)

congelado_cc_test(
    name = "workflow",
    srcs = ["test.cc"],
    deps = [
        ":core_workflow",
        "//plugins/memory:memory_backends",
    ],
    linkopts = ["-lcrypto", "-lssl"],
)
