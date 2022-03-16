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
    #"usr/lib/x86_64-linux-gnu/libexpat.so",
    #"usr/lib/x86_64-linux-gnu/libexpat.so.1",
    #"usr/lib/x86_64-linux-gnu/libexpat.so.1.8.1",
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
    "usr/lib/x86_64-linux-gnu/libz.so.1",
    "usr/lib/x86_64-linux-gnu/libz.so.1.2.11",
  ],
  hdrs = ["usr/include/zconf.h", "usr/include/zlib.h"],
  strip_include_prefix = "usr/include",
  visibility = ["//visibility:public"],
)

cc_library(
  name = "python-dev",
  srcs = [
    "usr/lib/x86_64-linux-gnu/libpython3.9.so.1",
    "usr/lib/x86_64-linux-gnu/libpython3.9.so.1.0",
    "usr/lib/x86_64-linux-gnu/libpython3.9.so",
    # "usr/lib/x86_64-linux-gnu/libpython3.9.a",
    "usr/lib/python3.9/config-3.9-x86_64-linux-gnu/libpython3.9-pic.a",
    "usr/lib/python3.9/config-3.9-x86_64-linux-gnu/libpython3.9.so",
    "usr/lib/python3.9/config-3.9-x86_64-linux-gnu/libpython3.9.a",
  ],
  hdrs = glob([
    "usr/include/python3.9/*.h",
    "usr/include/python3.9/**/*.h",
    ]),
  deps = [":zlib", "expat"],
  includes = ["usr/include/python3.9"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "libllvm",
  srcs = [
    "usr/lib/llvm-13/lib/libLLVM.so",
    "usr/lib/llvm-13/lib/libLLVM-13.so",
    "usr/lib/llvm-13/lib/libLLVM-13.so.1",
  ],
  hdrs = glob(["usr/lib/llvm-13/include/llvm-c/*.h"]),
  includes = ["usr/lib/llvm-13/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "libclang",
  srcs = glob([
    "usr/lib/llvm-13/lib/libclang*",
  ]) + [
    # "usr/lib/x86_64-linux-gnu/libclang-13.so",
    # "usr/lib/x86_64-linux-gnu/libclang-13.so.1",
    "usr/lib/x86_64-linux-gnu/libclang-13.so.13",
    "usr/lib/x86_64-linux-gnu/libclang-13.so.13.0.0",
  ],
  deps = [":libllvm"],
  hdrs = glob(["usr/lib/llvm-13/include/clang-c/*.h"]),
  includes = ["usr/lib/llvm-13/include"],
  visibility = ["//visibility:public"],
)

py_runtime(
  name = "python-3.9.7",
  files = glob(["usr/lib/python3.9/*"]),
  interpreter = "usr/bin/python3.9",
  python_version = "PY3",
)


py_runtime_pair(
  name = "py_runtime",
  py3_runtime = ":python-3.9.7",
)

