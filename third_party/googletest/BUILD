package(default_visibility=["//visibility:public"])
licenses(["notice"])

config_setting(
    name="windows",
    constraint_values=["@bazel_tools//platforms:windows"],
)

config_setting(
    name="has_absl",
    values={"define": "absl=1"},
)

# Library that defines the FRIEND_TEST macro.
cc_library(
    name="gtest_prod",
    hdrs=["include/gtest/gtest_prod.h"],
    includes=["include"],
)

# Google Test including Google Mock
cc_library(
    name="gtest",
    srcs=glob(
        include=[
            "src/*.cc",
            "src/*.h",
            "include/gtest/**/*.h",
        ],
        exclude=[
            "src/gtest-all.cc",
            "src/gtest_main.cc",
        ],
    ),
    hdrs=glob([
        "include/gtest/*.h",
    ]),
    copts=select({
        ":windows": [],
        "//conditions:default": ["-pthread"],
    }),
    defines=select({
        ":has_absl": ["GTEST_HAS_ABSL=1"],
        "//conditions:default": [],
    }),
    includes=[
        ".",
        "include",
    ],
    linkopts=select({
        ":windows": [],
        "//conditions:default": ["-pthread"],
    }),
    deps=select({
        ":has_absl": [
            "@com_google_absl//absl/debugging:failure_signal_handler",
            "@com_google_absl//absl/debugging:stacktrace",
            "@com_google_absl//absl/debugging:symbolize",
            "@com_google_absl//absl/strings",
            "@com_google_absl//absl/types:optional",
            "@com_google_absl//absl/types:variant",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name="gtest_main",
    srcs=["src/gtest_main.cc"],
    deps=[":gtest"],
)
