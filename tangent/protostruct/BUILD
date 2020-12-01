load(
  "@rules_cc//cc:defs.bzl",
  "cc_binary",
  "cc_library",
  "cc_proto_library",
  "cc_test",
)
load("@rules_python//python:defs.bzl", "py_binary", "py_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
load("@pip_deps//:requirements.bzl", "requirement")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "pbwire",
  srcs = ["pbwire.cc"],
  hdrs = ["pbwire.h"],
  linkstatic = True,
  deps = ["//tangent/util"],
)

cc_library(
  name = "cereal_utils",
  hdrs = ["cereal_utils.h"],
  deps = [
    "//third_party/cereal",
  ],
)

cc_test(
  name = "pbwire-test",
  srcs = ["pbwire-test.cc"],
  deps = [
    ":pbwire",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

proto_library(
  name = "descriptor_extensions_proto",
  srcs = ["descriptor_extensions.proto"],
  deps = ["@com_google_protobuf//:descriptor_proto"],
)

cc_proto_library(
  name = "descriptor_extensions_cc_proto",
  deps = ["descriptor_extensions_proto"],
)

cc_binary(
  name = "protostruct-header",
  srcs = [
    "protostruct-main.cc",
  ],
  deps = [
    ":descriptor_extensions_cc_proto",
    "//argue",
    "@system//:libclang",
    "@system//:python-dev",
  ],
)

py_proto_library(
  name = "descriptor_extensions_py_proto",
  srcs = [
    "descriptor_extensions.proto",
  ],
  deps = ["@com_google_protobuf//:protobuf_python"],
)

py_library(
  name = "protostruct_gen",
  srcs = [
    "__init__.py",
    "gen.py",
  ],
  data = glob(["templates/*.jinja2"]),
  deps = [
    ":descriptor_extensions_py_proto",
    requirement("jinja2"),
    requirement("markupsafe"),
  ],
)

py_binary(
  name = "make_pyzip",
  srcs = ["make_pyzip.py"],
  deps = [":protostruct_gen"],
)

genrule(
  name = "gen-pyzip",
  srcs = ["gen.py"],
  outs = [
    "pyzip_exe_body.zip",
  ],
  cmd = """
    $(location :make_pyzip) \
      -o $(RULEDIR)/pyzip_exe_body.zip \
      -m $(location gen.py)""",
  tools = [
    ":make_pyzip",
  ],
)

genrule(
  name = "gen-protostruct",
  srcs = [
    ":protostruct-header",
    "pyzip_exe_body.zip",
  ],
  outs = [
    "protostruct",
  ],
  cmd = """
    cat $(location protostruct-header) $(location pyzip_exe_body.zip) \
      > $@ \
    && chmod 0600 $@
  """,
)

genrule(
  name = "gen-test_messages.pb3",
  srcs = [
    "test/test_messages.h",
    "test/test_in.proto",
  ],
  outs = [
    "test/test_messages.pb3",
  ],
  cmd = """\
    $(execpath protostruct) tangent/protostruct/test/test_messages.h \
       --step compile \
       --proto-path tangent/protostruct/test \
       --proto-in test_in.proto \
       --outfile $@ \
       -- -I . \
    """,
  tools = ["protostruct"],
)

genrule(
  name = "gen-test_messages.star",
  srcs = [
    "test/test_messages.pb3",
  ],
  outs = [
    "test/test_messages.proto",
    "test/test_messages.cereal.h",
    "test/test_messages.pbwire.h",
    "test/test_messages.pbwire.c",
    "test/test_messages.pb2c.h",
    "test/test_messages.pb2c.cc",
  ],
  cmd = """\
    $(execpath protostruct) --step gen \
      --outfile $< \
      --proto-out $(GENDIR) \
      --cpp-out $(GENDIR) \
  """,
  tools = ["protostruct"],
)

proto_library(
  name = "test_messages_proto",
  srcs = ["test/test_messages.proto"],
)

cc_proto_library(
  name = "test_messages_cc_proto",
  deps = ["test_messages_proto"],
)

cc_library(
  name = "test-messages",
  srcs = [
    "test/test_messages.cereal.h",
    "test/test_messages.h",
    "test/test_messages.pb2c.cc",
    "test/test_messages.pb2c.h",
    "test/test_messages.pbwire.c",
    "test/test_messages.pbwire.h",
  ],
  linkstatic = True,
  deps = [
    ":cereal_utils",
    ":pbwire",
    ":test_messages_cc_proto",
    "//third_party/cereal",
  ],
)

cc_test(
  name = "devtest3",
  srcs = ["devtest.cc"],
  deps = [
    ":cereal_utils",
    ":test-messages",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)
