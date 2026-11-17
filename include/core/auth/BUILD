load("//bazel:build_defs.bzl", "congelado_module_library")

# core_auth: Argon2id password hashing + JWT + sessions + RBAC.
# libsodium (BCR) supplies the crypto primitives — no hand-written crypto.
congelado_module_library(
    name = "core_auth",
    primary_interface = "base.cppm",
    partitions = [
        "auth_status.cppm",
        "password_hash.cppm",
        "password_hasher.cppm",
        "token.cppm",
        "jwt_signer.cppm",
        "session_store.cppm",
        "permission.cppm",
        "role.cppm",
        "rbac.cppm",
        "api_key.cppm",
        "auth_runtime.cppm",
        "auth.cppm",
        "auth_builder.cppm",
    ],
    deps = ["@libsodium//:libsodium"],
    visibility = ["//visibility:public"],
)
