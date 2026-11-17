"""Module extension providing the `std` named module to all module targets.

The libstdc++ module interface source (`bits/std.cc`, shipped with the host
toolchain) is copied into the generated repo and compiled by Bazel's configured
C++ toolchain with `-std=gnu++26`, so its BMI matches every consumer built by
congelado_module_library (which uses the same flag via congelado_cxx26_copts).
"""

def _system_libstdcxx_repo(repo_ctx):
    found = repo_ctx.execute([
        "bash",
        "-lc",
        "ls /usr/include/c++/*/bits/std.cc 2>/dev/null | head -1",
    ])
    std_cc = found.stdout.strip()
    if not std_cc:
        fail("system_libstdcxx: could not find bits/std.cc under /usr/include/c++")

    repo_ctx.symlink(std_cc, "std.cppm")

    repo_ctx.file(
        "BUILD.bazel",
        content = """
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "std",
    module_interfaces = ["std.cppm"],
    hdrs = ["std.cppm"],
    copts = ["-std=gnu++26", "-fPIC"],
    features = ["cpp_modules"],
    visibility = ["//visibility:public"],
)
""",
    )

_system_libstdcxx_repo_rule = repository_rule(
    implementation = _system_libstdcxx_repo,
    local = True,
)

def _system_libstdcxx_extension(module_ctx):
    _system_libstdcxx_repo_rule(name = "system_libstdcxx")

system_libstdcxx = module_extension(
    implementation = _system_libstdcxx_extension,
)
