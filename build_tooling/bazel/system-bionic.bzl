load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:defs.bzl", "py_runtime", "py_runtime_pair")

filegroup(
  name = "lsb-release",
  srcs = ["etc/lsb-release"],
)

cc_library(
  name = "python-dev",
  srcs = [
    "usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/libpython3.6.so",
    "usr/lib/x86_64-linux-gnu/libpython3.6m.so.1.0",
    "usr/lib/x86_64-linux-gnu/libpython3.6m.so.1",
  ],
  hdrs = glob(["usr/include/python3.6m/*.h"]),
  includes = ["usr/include/python3.6m"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "libllvm",
  srcs = [
    "usr/lib/llvm-8/lib/libLLVM.so",
    "usr/lib/llvm-8/lib/libLLVM-8.so",
  ],
  hdrs = glob(["usr/lib/llvm-8/include/llvm-c/*.h"]),
  includes = ["usr/lib/llvm-8/include"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "libclang",
  srcs = [
    "usr/lib/llvm-8/lib/libclang-8.0.0.so",
    "usr/lib/llvm-8/lib/libclang-8.so",
    "usr/lib/llvm-8/lib/libclang-8.so.1",
    "usr/lib/x86_64-linux-gnu/libclang-8.so",
    "usr/lib/x86_64-linux-gnu/libclang-8.so.1",
  ],
  deps = ["libllvm"],
  hdrs = glob(["usr/lib/llvm-8/include/clang-c/*.h"]),
  includes = ["usr/lib/llvm-8/include"],
  visibility = ["//visibility:public"],
)

py_runtime(
  name = "python-3.6.6",
  files = glob(["usr/lib/python3.6/*"]),
  interpreter = "usr/bin/python3.6",
  python_version = "PY3",
)

py_runtime(
  name = "python-2.7.17",
  files = glob(["usr/lib/python2.7/*"]),
  interpreter = "usr/bin/python2.7",
  python_version = "PY2",
)

py_runtime_pair(
  name = "py_runtime",
  py2_runtime = ":python-2.7.17",
  py3_runtime = ":python-3.6.6",
)
