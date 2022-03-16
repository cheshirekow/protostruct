load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:defs.bzl", "py_runtime", "py_runtime_pair")

filegroup(
  name = "lsb-release",
  srcs = ["etc/lsb-release"],
)

cc_library(
  name = "expat",
  includes = ["usr/include"],
  srcs = [
    "usr/lib/x86_64-linux-gnu/libexpat.a",
    "usr/lib/x86_64-linux-gnu/libexpat.so",
  ],
  hdrs = ["usr/include/expat.h"],
  strip_include_prefix = "usr/include",
  visibility = ["//visibility:public"],
)

cc_library(
  name = "zlib",
  includes = ["usr/include"],
  srcs = [
    "usr/lib/x86_64-linux-gnu/libz.a",
    "usr/lib/x86_64-linux-gnu/libz.so",
  ],
  hdrs = ["usr/include/zconf.h", "usr/include/zlib.h"],
  strip_include_prefix = "usr/include",
  visibility = ["//visibility:public"],
)

cc_library(
  name = "python-dev",
  srcs = [
    "usr/lib/x86_64-linux-gnu/libpython3.8.so.1",
    "usr/lib/x86_64-linux-gnu/libpython3.8.so.1.0",
    "usr/lib/x86_64-linux-gnu/libpython3.8.so",
    # "usr/lib/x86_64-linux-gnu/libpython3.8.a",
    "usr/lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8.so",
    # "usr/lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8.a",
    # "usr/lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8-pic.a",
  ],
    hdrs = glob([
    "usr/include/python3.8/*.h",
    "usr/include/python3.8/**/*.h",
    "usr/include/x86_64-linux-gnu/python3.8/*.h"
    ]),
  deps = [":zlib", "expat"],
  includes = ["usr/include/python3.8"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "libllvm",
  srcs = [
    "usr/lib/llvm-11/lib/libLLVM.so",
    "usr/lib/llvm-11/lib/libLLVM-11.so",
  ],
  hdrs = glob(["usr/lib/llvm-11/include/llvm-c/*.h"]),
  includes = ["usr/lib/llvm-11/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "libclang",
  srcs = [
    "usr/lib/llvm-11/lib/libclang-11.0.0.so",
    "usr/lib/llvm-11/lib/libclang-11.so",
    "usr/lib/llvm-11/lib/libclang-11.so.1",
    "usr/lib/x86_64-linux-gnu/libclang-11.so",
    "usr/lib/x86_64-linux-gnu/libclang-11.so.1",
  ],
  deps = ["libllvm"],
  hdrs = glob(["usr/lib/llvm-11/include/clang-c/*.h"]),
  includes = ["usr/lib/llvm-11/include"],
  visibility = ["//visibility:public"],
)

py_runtime(
  name = "python-3.8",
  files = glob(["usr/lib/python3.8/*"]),
  interpreter = "usr/bin/python3.8",
  python_version = "PY3",
)


py_runtime_pair(
  name = "py_runtime",
  py3_runtime = ":python-3.8",
)

