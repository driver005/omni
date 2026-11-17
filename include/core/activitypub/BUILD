load("//bazel:build_defs.bzl", "congelado_module_library")

# HTTP Message Signatures (RFC 9421) via OpenSSL — no hand-written crypto.
congelado_module_library(
    name = "core_activitypub",
    primary_interface = "base.cppm",
    partitions = [
        "activity_type.cppm",
        "actor.cppm",
        "activity.cppm",
        "inbox.cppm",
        "outbox.cppm",
        "http_signature.cppm",
        "runtime.cppm",
        "activitypub.cppm",
        "builder.cppm",
    ],
    deps = ["@openssl//:crypto"],
    visibility = ["//visibility:public"],
)
