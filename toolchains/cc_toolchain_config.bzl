load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
     "feature", "flag_group", "flag_set", "tool_path")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")

def _impl(ctx):
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = "clang_windows_x64",
        host_system_name = "x86_64-pc-linux-gnu",
        target_system_name = "x86_64-w64-windows-gnu",
        target_cpu = "x64_windows",
        compiler = "clang",
        abi_version = "gnu",
        abi_libc_version = "mingw",
        features = [
            # --- C++ flags ---
            feature(
                name = "cxx_compile_flags",
                enabled = True,
                flag_sets = [
                    flag_set(
                        actions = ["c++-compile"],
                        flag_groups = [flag_group(
                            flags = [
                                "--target=x86_64-w64-windows-gnu",
                                "-stdlib=libstdc++",
                                "-std=c++20",
                                "-fPIC",
                            ],
                        )],
                    ),
                ],
            ),
            # --- C flags ---
            feature(
                name = "c_compile_flags",
                enabled = True,
                flag_sets = [
                    flag_set(
                        actions = ["c-compile"],
                        flag_groups = [flag_group(
                            flags = [
                                "--target=x86_64-w64-windows-gnu",
                                "-fPIC",
                            ],
                        )],
                    ),
                ],
            ),
        ],
        tool_paths = [
            tool_path(name = "gcc", path = "/usr/bin/clang-cl"),
            tool_path(name = "ld", path = "/usr/bin/lld"),
            tool_path(name = "ar", path = "/usr/bin/llvm-ar"),
            tool_path(name = "cpp", path = "/usr/bin/clang-cpp"),
            tool_path(name = "nm", path = "/usr/bin/llvm-nm"),
            tool_path(name = "objdump", path = "/usr/bin/llvm-objdump"),
            tool_path(name = "strip", path = "/usr/bin/llvm-strip"),
        ],
        cxx_builtin_include_directories = [
            "/usr/lib/gcc/x86_64-w64-mingw32/9.3-win32/include/c++",
            "/usr/lib/gcc/x86_64-w64-mingw32/9.3-win32/include/c++/x86_64-w64-mingw32
            "/usr/lib/llvm-20/lib/clang/20/include",
        ]
    )

clang_windows_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
)
