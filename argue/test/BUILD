package(default_visibility = ["//visibility:public"])

cc_test(
  name = "argue-assert_test",
  srcs = ["assert_test.cc"],
  deps = [
    "//argue",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "argue-arg_test",
  srcs = ["arg_test.cc"],
  deps = [
    "//argue",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "argue-keyword_test",
  srcs = ["keyword_test.cc"],
  deps = [
    "//argue",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "argue-parse_test",
  srcs = ["parse_test.cc"],
  deps = [
    "//argue",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

cc_test(
  name = "argue-util_test",
  srcs = ["util_test.cc"],
  deps = [
    "//argue",
    "@gtest",
    "@gtest//:gtest_main",
  ],
)

# NOTE(josh): see
# https://docs.bazel.build/versions/master/skylark/testing.html
sh_test(
  name = "argue-execution_test",
  srcs = ["execution_tests.py"],
  args = [
    "--exe-path",
    "$(location //argue/examples:argparse-example)",
  ],
  data = ["//argue/examples:argparse-example"],
)
