load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

# -------------------------------------------------------------------
# Top-level Unity plugin (.so)
# -------------------------------------------------------------------
cc_binary(
    name = "nc_unity",
    srcs = ["nc_unity_adapter.h"],
    deps = select({
        "@platforms//os:android": ["//connections/unity:nc_unity_jni_bridge"],
        "//conditions:default": ["//connections/unity:nc_unity_native_adapter"],
    }),
    linkshared = True,  # produces a true standalone libnc_unity.so
    visibility = ["//visibility:public"],
)

# -------------------------------------------------------------------
# Native (desktop / simulator) implementation
# -------------------------------------------------------------------
cc_library(
    name = "nc_unity_native_adapter",
    srcs = ["nc_unity_adapter.cpp"],
    hdrs = ["nc_unity_adapter.h"],
    copts = ["-Wall", "-Werror", "-fPIC"],
    deps = ["//connections/c:nc"],
    visibility = ["//connections/unity:__subpackages__"],
)

# -------------------------------------------------------------------
# JNI bridge (Android)
# -------------------------------------------------------------------
cc_library(
    name = "nc_unity_jni_bridge",
    srcs = ["nc_unity_adapter_android_jni.cc"],
    hdrs = ["nc_unity_adapter.h"],
    copts = ["-fPIC"],
    deps = [
        #"@android_ndk//:jni_headers",
    ],
    linkopts = ["-llog"],  # optional, for logcat logging
    visibility = ["//connections/unity:__subpackages__"],
)