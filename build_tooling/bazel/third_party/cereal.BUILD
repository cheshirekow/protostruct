load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "cereal",
  hdrs = glob([
    "include/*.hpp",
    "include/**/*.h",
    "include/**/*.hpp",
  ]),
  includes = ["include"],
)
