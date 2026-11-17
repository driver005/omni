"""Back-compat shim: include/src/BUILD loads congelado_cc_binary from here."""

load("//bazel:build_defs.bzl", _congelado_cc_binary = "congelado_cc_binary")

# Re-export under the name include/src/BUILD expects.
congelado_cc_binary = _congelado_cc_binary
