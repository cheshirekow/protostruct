load("@rules_cc//cc:defs.bzl", "cc_library")
load("@bazel_skylib//rules:common_settings.bzl", "bool_flag")

package(default_visibility = ["//visibility:public"])

toolchain(
  name = "py_toolchain",
  toolchain = "@system//:py_runtime",
  toolchain_type = "@bazel_tools//tools/python:toolchain_type",
)

bool_flag(
  name = "with_system",
  build_setting_default = True,
)

config_setting(
  name = "config_with_system",
  flag_values = {
    ":with_system": "True",
  },
)
