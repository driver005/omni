# Minimal C++ rules for Bazel 9.x without rules_cc
# These are simplified Starlark implementations that wrap clang++

# CcInfo and CompilationContext providers
CcInfo = provider(
    fields = ["compilation_context", "linking_context"],
)

CompilationContext = provider(
    fields = ["includes", "defines"],
)

LinkingContext = provider(
    fields = ["linkopts", "libraries"],
)

def _compile_args(ctx, includes, defines, copts):
    args = ["clang++", "-std=c++20"]
    args.extend(["-I" + inc for inc in includes])
    args.extend(["-D" + d for d in defines])
    # Legacy BUILD files pass GCC's -fmodules-ts; clang uses -fmodules instead.
    args.extend([c for c in copts if c != "-fmodules-ts"])
    return args

def _dep_files(deps):
    files = []
    for dep in deps:
        files.extend(dep[DefaultInfo].files.to_list())
    return files

# cc_library rule
def _cc_library_impl(ctx):
    srcs = ctx.files.srcs
    hdrs = ctx.files.hdrs
    includes = ctx.attr.includes
    copts = ctx.attr.copts
    defines = ctx.attr.defines
    linkopts = ctx.attr.linkopts
    deps = ctx.attr.deps

    cmd = _compile_args(ctx, includes, defines, copts)

    objects = []
    for src in srcs:
        obj = ctx.actions.declare_file(src.basename + ".o")
        ctx.actions.run_shell(
            inputs = [src] + hdrs,
            outputs = [obj],
            command = " ".join(cmd + ["-c", src.path, "-o", obj.path]),
            mnemonic = "CcCompile",
        )
        objects.append(obj)

    lib = ctx.actions.declare_file("lib%s.a" % ctx.label.name)
    ctx.actions.run_shell(
        inputs = objects,
        outputs = [lib],
        command = "ar rcs %s %s" % (lib.path, " ".join([o.path for o in objects])),
        mnemonic = "CcArchive",
    )

    return [
        DefaultInfo(files = depset([lib] + _dep_files(deps))),
        CcInfo(
            compilation_context = CompilationContext(
                includes = includes,
                defines = defines,
            ),
            linking_context = LinkingContext(
                linkopts = linkopts,
                libraries = [lib.path],
            ),
        ),
    ]

cc_library = rule(
    implementation = _cc_library_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".cppm", ".cpp", ".cc", ".cxx", ".c++", ".C"]),
        "hdrs": attr.label_list(allow_files = [".hpp", ".h", ".hh", ".hxx", ".h++", ".H"]),
        "includes": attr.string_list(),
        "copts": attr.string_list(),
        "defines": attr.string_list(),
        "linkopts": attr.string_list(),
        "deps": attr.label_list(),
    },
    provides = [CcInfo, DefaultInfo],
)

# cc_binary / cc_test rule
def _cc_binary_impl(ctx):
    srcs = ctx.files.srcs
    hdrs = ctx.files.hdrs
    includes = ctx.attr.includes
    copts = ctx.attr.copts
    defines = ctx.attr.defines
    linkopts = ctx.attr.linkopts
    deps = ctx.attr.deps

    cmd = _compile_args(ctx, includes, defines, copts)

    objects = []
    for src in srcs:
        obj = ctx.actions.declare_file(src.basename + ".o")
        ctx.actions.run_shell(
            inputs = [src] + hdrs,
            outputs = [obj],
            command = " ".join(cmd + ["-c", src.path, "-o", obj.path]),
            mnemonic = "CcCompile",
        )
        objects.append(obj)

    binary = ctx.actions.declare_file(ctx.label.name)
    dep_files = _dep_files(deps)
    ctx.actions.run_shell(
        inputs = objects + dep_files,
        outputs = [binary],
        command = " ".join(
            ["clang++"] + [o.path for o in objects] +
            [f.path for f in dep_files if f.basename.endswith(".a")] +
            linkopts + ["-o", binary.path]
        ),
        mnemonic = "CcLink",
    )

    return [DefaultInfo(files = depset([binary]), executable = binary)]

cc_binary = rule(
    implementation = _cc_binary_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".cppm", ".cpp", ".cc", ".cxx", ".c++", ".C"]),
        "hdrs": attr.label_list(allow_files = [".hpp", ".h", ".hh", ".hxx", ".h++", ".H"]),
        "includes": attr.string_list(),
        "copts": attr.string_list(),
        "defines": attr.string_list(),
        "linkopts": attr.string_list(),
        "deps": attr.label_list(),
    },
    provides = [DefaultInfo],
    executable = True,
)

cc_test = rule(
    implementation = _cc_binary_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".cppm", ".cpp", ".cc", ".cxx", ".c++", ".C"]),
        "hdrs": attr.label_list(allow_files = [".hpp", ".h", ".hh", ".hxx", ".h++", ".H"]),
        "includes": attr.string_list(),
        "copts": attr.string_list(),
        "defines": attr.string_list(),
        "linkopts": attr.string_list(),
        "deps": attr.label_list(),
    },
    provides = [DefaultInfo],
    test = True,
)
