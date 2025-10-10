load(":cc_toolchain_config.bzl", "clang_windows_toolchain_config")
load("@rules_cc//cc/toolchains:cc_toolchain.bzl", "cc_toolchain")

package(default_visibility = ["//visibility:public"])

# Platform definition
platform(
    name = "windows_x64",
    constraint_values = [
        "@platforms//os:windows",
        "@platforms//cpu:x86_64",
    ],
)

# Toolchain configuration
clang_windows_toolchain_config(name = "clang_windows_config")

cc_toolchain(
    name = "clang_windows_toolchain",
    toolchain_identifier = "clang_windows_x64",
    all_files = ":empty",
    ar_files = ":empty",
    as_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 1,
    toolchain_config = ":clang_windows_config",
)

filegroup(name = "empty", srcs = [])

toolchain(
    name = "clang_windows_toolchain_impl",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
    toolchain = ":clang_windows_toolchain",
    exec_compatible_with = [],
    target_compatible_with = ["@platforms//os:windows"],
)
