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

# cc_library rule
def _cc_library_impl(ctx):
    # Collect all source files
    srcs = ctx.files.srcs
    hdrs = ctx.files.hdrs
    
    # Get includes
    includes = ctx.attr.includes
    
    # Get copts
    copts = ctx.attr.copts
    
    # Get defines
    defines = ctx.attr.defines
    
    # Get linkopts
    linkopts = ctx.attr.linkopts
    
    # Get deps
    deps = ctx.attr.deps
    
    # Build compile command
    cmd = ["clang++", "-std=c++20", "-fmodules-ts", "-c"]
    cmd.extend(["-I" + inc for inc in includes])
    cmd.extend(["-D" + d for d in defines])
    cmd.extend(copts)
    
    # Compile each source file
    for src in srcs:
        out = "%s.o" % src.basename
        ctx.actions.run(
            inputs = [src] + hdrs,
            outputs = [out],
            command = " ".join(cmd + [src.path, "-o", out]),
        )
    
    # Create static library
    lib_name = "lib%s.a" % ctx.label.name
    ctx.actions.run(
        inputs = ["%s.o" % src.basename for src in srcs],
        outputs = [lib_name],
        command = "ar rcs %s %s" % (lib_name, " ".join(["%s.o" % src.basename for src in srcs])),
    )
    
    return [
        DefaultInfo(files = depset([lib_name])),
        CcInfo(
            compilation_context = CompilationContext(
                includes = includes,
                defines = defines,
            ),
            linking_context = LinkingContext(
                linkopts = linkopts,
                libraries = [lib_name],
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

# cc_binary rule
def _cc_binary_impl(ctx):
    srcs = ctx.files.srcs
    hdrs = ctx.files.hdrs
    includes = ctx.attr.includes
    copts = ctx.attr.copts
    defines = ctx.attr.defines
    linkopts = ctx.attr.linkopts
    deps = ctx.attr.deps
    
    # Build compile command
    cmd = ["clang++", "-std=c++20", "-fmodules-ts"]
    cmd.extend(["-I" + inc for inc in includes])
    cmd.extend(["-D" + d for d in defines])
    cmd.extend(copts)
    
    # Compile each source file
    objs = []
    for src in srcs:
        out = "%s.o" % src.basename
        ctx.actions.run(
            inputs = [src] + hdrs,
            outputs = [out],
            command = " ".join(cmd + ["-c", src.path, "-o", out]),
        )
        objs.append(out)
    
    # Link
    out_name = ctx.label.name
    link_cmd = ["clang++"]
    link_cmd.extend(objs)
    link_cmd.extend(linkopts)
    
    # Add library dependencies
    for dep in deps:
        link_cmd.append(dep.path)
    
    ctx.actions.run(
        inputs = objs,
        outputs = [out_name],
        command = " ".join(link_cmd + ["-o", out_name]),
    )
    
    return [
        DefaultInfo(files = depset([out_name]), executable = out_name),
    ]

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

# cc_test rule
def _cc_test_impl(ctx):
    # Tests are just binaries with test_main
    return _cc_binary_impl(ctx)

cc_test = rule(
    implementation = _cc_test_impl,
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