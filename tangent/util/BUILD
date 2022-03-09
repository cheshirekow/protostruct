load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "util",
  srcs = [
    "base64.cc",
    "colormap.cc",
    "exception.cc",
    "linked.cc",
    "redblack.cc",
    "refcounted.cc",
    "stack_trace.cc",
    "stringutil.cc",
  ],
  hdrs = [
    "array_stack.h",
    "backports.h",
    "base64.h",
    "binary_literal.h",
    "bitset.h",
    "colormap.h",
    "container_of.h",
    "endian.h",
    "exception.h",
    "fallthrough.h",
    "fixed_string_stream.h",
    "hash.h",
    "inline_math.h",
    "linked.h",
    "linked.hpp",
    "null_out.h",
    "nullstream.h",
    "range.hpp",
    "redblack.h",
    "refcounted.h",
    "set_operations.hpp",
    "stack_trace.h",
    "stdio_filebuf.h",
    "stringutil.h",
    "timespec.h",
    "type_string.h",
  ],
)

cc_test(
  name = "array_stack-test",
  srcs = ["array_stack_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "base64-test",
  srcs = ["base64_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "bitset_test",
  srcs = ["bitset_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "colormap-test",
  srcs = ["colormap_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
    "@system//:libX11",
  ],
)

cc_test(
  name = "endian-test",
  srcs = ["endian_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "exception-test",
  srcs = ["exception_test.cc"],
  deps = [
    ":util",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "linked_test",
  srcs = ["linked_test.cc"],
  deps = [
    ":util",
    "//third_party:fmt",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "redblack_test",
  srcs = ["redblack_test.cc"],
  deps = [
    ":util",
    "//third_party:fmt",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "type_string_test",
  srcs = ["type_string_test.cc"],
  deps = [
    ":util",
    "//third_party:fmt",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "fixed_string_stream_test",
  srcs = ["fixed_string_stream_test.cc"],
  deps = [
    ":util",
    "//third_party:fmt",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_library(
  name = "svg_artist",
  srcs = ["svg_artist.cc"],
  hdrs = ["svg_artist.h"],
  deps = [
    "//third_party:fmt",
    "@system//:eigen3",
  ],
)
