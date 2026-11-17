"""Platform conditionals used by copts.bzl.

Each helper returns a Bazel `select(...)` so option lists compose in attributes
via `+` (TF-style), instead of hard-coding the host platform.
"""

def if_linux(x):
    return select({
        "@platforms//os:linux": x,
        "//conditions:default": [],
    })

def if_x86_64(x):
    return select({
        "@platforms//cpu:x86_64": x,
        "//conditions:default": [],
    })
