load(
  "@rules_cc//cc:defs.bzl",
  "cc_binary",
  "cc_library",
)

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "tjson",
  srcs = [
    "emit.c",
    "parse.c",
    "tjson.c",
  ],
  hdrs = [
    "emit.h",
    "ostream.h",
    "parse.h",
    "tjson.h",
  ],
  deps = ["//tangent/util"],
)

cc_library(
  name = "cpp",
  srcs = ["cpputil.cc"],
  hdrs = ["cpputil.h"],
  deps = [":tjson"],
)

cc_binary(
  name = "exe",
  srcs = ["main.cc"],
  deps = [
    ":tjson",
    "//argue",
  ],
)
