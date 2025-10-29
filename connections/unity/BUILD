load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "nc_unity",
    srcs = ["nc_unity_adapter.cpp"],
    hdrs = ["nc_unity_adapter.h"],
    copts = [
        "-Wall",
        "-Werror",
        "-fPIC",  # for shared lib
    ],
    deps = [
        "//connections/c:nc",  # or :libnc_prebuilt if prebuilt
    ],
)