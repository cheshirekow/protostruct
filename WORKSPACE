load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//build_tooling/bazel:common.bzl", "suite_repository")

# rules_cc defines rules for generating C++ code from Protocol Buffers.
http_archive(
  name = "rules_cc",
  sha256 = "35f2fb4ea0b3e61ad64a369de284e4fbbdcdba71836a5555abb5e194cf119509",
  strip_prefix = "rules_cc-624b5d59dfb45672d4239422fa1e3de1822ee110",
  urls = [
    "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
    "https://github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
  ],
)

# rules_java defines rules for generating Java code from Protocol Buffers.
http_archive(
  name = "rules_java",
  sha256 = "ccf00372878d141f7d5568cedc4c42ad4811ba367ea3e26bc7c43445bbc52895",
  strip_prefix = "rules_java-d7bf804c8731edd232cb061cb2a9fe003a85d8ee",
  urls = [
    "https://mirror.bazel.build/github.com/bazelbuild/rules_java/archive/d7bf804c8731edd232cb061cb2a9fe003a85d8ee.tar.gz",
    "https://github.com/bazelbuild/rules_java/archive/d7bf804c8731edd232cb061cb2a9fe003a85d8ee.tar.gz",
  ],
)

http_archive(
  name = "com_google_protobuf",
  sha256 = "9b4ee22c250fe31b16f1a24d61467e40780a3fbb9b91c3b65be2a376ed913a1a",
  strip_prefix = "protobuf-3.13.0",
  urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.13.0.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
  name = "rules_python",
  sha256 = "a30abdfc7126d497a7698c29c46ea9901c6392d6ed315171a6df5ce433aa4502",
  strip_prefix = "rules_python-0.6.0",
  url = "https://github.com/bazelbuild/rules_python/archive/0.6.0.tar.gz",
)

load("@rules_cc//cc:repositories.bzl", "rules_cc_dependencies")

rules_cc_dependencies()

load(
  "@rules_java//java:repositories.bzl",
  "rules_java_dependencies",
  "rules_java_toolchains",
)

rules_java_dependencies()

rules_java_toolchains()

load(
  "@rules_proto//proto:repositories.bzl",
  "rules_proto_dependencies",
  "rules_proto_toolchains",
)

rules_proto_dependencies()

rules_proto_toolchains()

suite_repository(name = "suite")

load("@suite//:defs.bzl", "DISTRIB_CODENAME")

new_local_repository(
  name = "system",
  build_file = "build_tooling/bazel/system-{}.bzl".format(DISTRIB_CODENAME),
  path = "/",
)

load("@rules_python//python:pip.bzl", "pip_install")

pip_install(
  name = "pip_deps",
  requirements = "//:pip-requirements.txt",
)

register_toolchains("//:py_toolchain")

http_archive(
  name = "fmt",
  build_file = "//build_tooling/bazel:third_party/fmt.BUILD",
  sha256 = "3d794d3cf67633b34b2771eb9f073bde87e846e0d395d254df7b211ef1ec7346",
  strip_prefix = "fmt-8.1.1",
  urls = [
    "https://github.com/fmtlib/fmt/archive/refs/tags/8.1.1.tar.gz",
  ],
)

http_archive(
  name = "gflags",
  sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
  strip_prefix = "gflags-2.2.2",
  urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
  name = "glog",
  repo_mapping = {
    "@com_github_gflags_gflags": "@gflags",
  },
  sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
  strip_prefix = "glog-0.5.0",
  urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

http_archive(
  name = "re2",
  patches = ["//build_tooling/bazel:third_party/re2-BUILD.patch"],
  sha256 = "9c1e6acfd0fed71f40b025a7a1dabaf3ee2ebb74d64ced1f9ee1b0b01d22fd27",
  strip_prefix = "re2-2022-02-01",
  urls = ["https://github.com/google/re2/archive/refs/tags/2022-02-01.tar.gz"],
)

http_archive(
  name = "gtest",
  sha256 = "b4870bf121ff7795ba20d20bcdd8627b8e088f2d1dab299a031c1034eddc93d5",
  strip_prefix = "googletest-release-1.11.0",
  urls = [
    "https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz",
  ],
)

http_archive(
  name = "cereal",
  build_file = "//build_tooling/bazel:third_party/cereal.BUILD",
  sha256 = "16a7ad9b31ba5880dac55d62b5d6f243c3ebc8d46a3514149e56b5e7ea81f85f",
  strip_prefix = "cereal-1.3.2",
  urls = [
    "https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.tar.gz"
  ]
)