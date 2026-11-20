load("//bazel:build_defs.bzl", "congelado_cc_test", "congelado_module_library")

# core_adapters — real outbound HTTP adapters (Stripe / Tinybird / SMTP / webmentions).
# Own directory, own target. Main agent wires it into the app; nothing shared is touched here.
congelado_module_library(
    name = "core_adapters",
    primary_interface = "base.cppm",
    partitions = [
        "config.cppm",
        "json.cppm",
        "outbox_hook.cppm",
        "http.cppm",
        "client_bridge.cppm",
        "stripe.cppm",
        "tinybird.cppm",
        "smtp.cppm",
        "webmention.cppm",
    ],
    deps = [
        "//include/core/client:core_client",
        "//include/interfaces:interfaces",
    ],
    visibility = ["//visibility:public"],
)

congelado_cc_test(
    name = "adapters",
    srcs = ["test.cc"],
    deps = [":core_adapters"],
    linkopts = ["-lcrypto", "-lssl"],
)
