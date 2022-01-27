load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "util",
  hdrs = [
    "fixed_string_stream.h",
  ],
)

cc_test(
  name = "fixed_string_stream_test",
  srcs = ["fixed_string_stream_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
    "@system//:fmt",
  ],
)

