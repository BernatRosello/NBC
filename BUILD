load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_compile_commands",

    # Specify the targets of interest.
    # For example, specify a dict of targets and any flags required to build.
    targets = {
        "//connections:core_types" : "",
        "//connections:core_test" : "",
        "//connections:core" : "",
    },
    # No need to add flags already in .bazelrc. They're automatically picked up.
    # If you don't need flags, a list of targets is also okay, as is a single target string.
    # Wildcard patterns, like //... for everything, *are* allowed here, just like a build.
      # As are additional targets (+) and subtractions (-), like in bazel query https://docs.bazel.build/versions/main/query.html#expressions
    # And if you're working on a header-only library, specify a test or binary target that compiles it.
)


#   CPU Value       Platform
# armeabi-v7a     @platforms//cpu:armv7
# arm64-v8a       @platforms//cpu:arm64
# x86             @platforms//cpu:x86_32
# x86_64          @platforms//cpu:x86_64
platform(
    name = "aarmv7",
    visibility = ["//visibility:public"],
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:armv7",
    ],
)

platform(
    name = "aarm64",
    visibility = ["//visibility:public"],
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:arm64",
    ],
)

platform(
    name = "ax86_32",
    visibility = ["//visibility:public"],
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:x86_32",
    ],
)

platform(
    name = "ax86_64",
    visibility = ["//visibility:public"],
    constraint_values = [
        "@platforms//os:android",
        "@platforms//cpu:x86_64",
    ],
)