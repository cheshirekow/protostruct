load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "util",
  srcs = [
    "exception.cc",
    "stringutil.cc",
  ],
  hdrs = [
    "array_stack.h",
    "container_of.h",
    "exception.h",
    "fallthrough.h",
    "fixed_string_stream.h",
    "hash.h",
    "nullstream.h",
    "stdio_filebuf.h",
    "stringutil.h",
    "type_string.h",
  ],
)

cc_test(
  name = "array_stack-test",
  srcs = ["array_stack_test.cc"],
  deps = [
    ":util",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "exception-test",
  srcs = ["exception_test.cc"],
  deps = [
    ":util",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "fixed_string_stream_test",
  srcs = ["fixed_string_stream_test.cc"],
  deps = [
    ":util",
    "@fmt",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "hash-test",
  srcs = ["hash_test.cc"],
  deps = [
    ":util",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "type_string_test",
  srcs = ["type_string_test.cc"],
  deps = [
    ":util",
    "@fmt",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)
