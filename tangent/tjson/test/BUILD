load(
  "@rules_cc//cc:defs.bzl",
  "cc_test",
)

package(default_visibility = ["//visibility:public"])

cc_test(
  name = "lexer_test",
  srcs = ["lexer_test.cc"],
  deps = [
    "//tangent/tjson",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "ostream_test",
  srcs = ["ostream_test.cc"],
  deps = [
    "//tangent/tjson:cpp",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "parser_test",
  srcs = ["parser_test.cc"],
  deps = [
    "//tangent/tjson",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "string_test",
  srcs = ["string_test.cc"],
  deps = [
    "//tangent/tjson",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)
