package(default_visibility = ["//visibility:public"])

cc_library(
  name = "argue",
  srcs = [
    "action.cc",
    "exception.cc",
    "glog.cc",
    "json_dump.cc",
    "kwargs.cc",
    "parse.cc",
    "parser.cc",
  ],
  hdrs = [
    "action.h",
    "action.tcc",
    "argue.h",
    "exception.h",
    "glog.h",
    "json_dump.h",
    "keywords.h",
    "keywords.tcc",
    "kwargs.h",
    "kwargs.tcc",
    "parse.h",
    "parse.tcc",
    "parser.h",
    "parser.tcc",
    "storage_model.h",
    "storage_model.tcc",
    "util.h",
  ],
  deps = [
    "//tangent/util",
    "@fmt",
    "@glog",
  ],
)

