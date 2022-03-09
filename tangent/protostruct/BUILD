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
load(":tools.bzl", "protostruct_compile", "protostruct_gen")

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
  srcs = [
    "gen.py",
    ":descriptor_extensions_py_proto",
  ],
  outs = [
    "pyzip_exe_body.zip",
  ],
  cmd = """
    $(location :make_pyzip) \
      -o $(RULEDIR)/pyzip_exe_body.zip \
      -m $(location gen.py) \
      -b $(GENDIR)""",
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

proto_library(
  name = "test_messages_proto_in",
  srcs = ["sync/test_messages.proto"],
  import_prefix = "tangent/protostruct/test",
  strip_import_prefix = "sync",
  deps = [":descriptor_extensions_proto"],
)

protostruct_compile(
  name = "protoc-test_messages",
  pb3_out = "test/test_messages.pb3",
  proto_sync = ":test_messages_proto_in",
  source_patterns = [r"tangent/.*\.h"],
  sourcefile = "test/test_messages.c",
  deps = ["test/test_messages.h"],
)

protostruct_gen(
  name = "protog-test_messages",
  basenames = ["test/test_messages"],
  fdset = "test/test_messages.pb3",
  templates = [
    "cpp-simple",
    "cereal",
    "pbwire",
    "pb2c",
    "proto",
    "recon",
  ],
)

proto_library(
  name = "test_messages_proto",
  srcs = ["test/test_messages.proto"],
  deps = [":descriptor_extensions_proto"],
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
    "test/test_messages-recon.h",
    "test/test_messages-simple.cc",
    "test/test_messages-simple.h",
  ],
  linkstatic = True,
  deps = [
    ":cereal_utils",
    ":pbwire",
    ":test_messages_cc_proto",
    "//tangent/tjson:cpp",
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

py_binary(
  name = "make-release",
  srcs = ["make_release.py"],
  main = "make_release.py",
)

genrule(
  name = "release",
  srcs = ["tools.bzl"],
  outs = ["protostruct-0.2.0dev1.tar.gz"],
  cmd = """
  $(location :make-release) \
    --protostruct-path $(location :protostruct) \
    --protostruct-header-path $(location :protostruct-header) \
    --tools-bzl-path $(location tools.bzl) \
    --outfile $@
  """,
  tools = [
    ":make-release",
    ":protostruct",
    ":protostruct-header",
  ],
)
