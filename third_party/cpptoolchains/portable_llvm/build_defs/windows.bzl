def cc_windows_dll(name, srcs = [], deps = [], **kwargs):
    native.cc_binary(
        name = name,
        srcs = srcs,
        deps = deps,
        linkshared = True,
        **kwargs
    )

windows = struct(cc_windows_dll = cc_windows_dll)