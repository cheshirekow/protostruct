load("@rules_python//python:defs.bzl", "py_binary", "py_test")

def protostruct_compile(
    name = None,
    sourcefile = None,
    deps = None,
    pb3_out = None,
    source_patterns = None,
    name_patterns = None,
    proto_sync = None,
    visibility = None,
    cflags = None):
  """Use protostruct to reverse out the message spec from existing structs

  Args:
    sourcefile: string of sourcefile to reverse
    proto_path: list[string] of directories to include on the protopath
      when constructing protos
    deps: cc dependencies that need to be in the sandbox when we compile
      (e.g. headers, etc)
    proto_sync: path to a .proto file to use as input for synchronization
    visiblity: visibility of the generated .proto and .pb3 files
    cflags: compilation flags to add to the clang command line
    enforce: add a test which verifies that the generated .proto matches
      the input proto from `proto_sync`
  """

  if deps == None:
    deps = []
  if cflags == None:
    cflags = []
  if source_patterns == None:
    source_patterns = []
  if name_patterns == None:
    name_patterns = []

  srcs = [sourcefile]
  outs = []

  cmdparts = [
    "$(execpath //tangent/protostruct:protostruct)",
    "compile",
    "$(location {})".format(sourcefile),
  ]

  if proto_sync:
    cmdparts += ["  --proto-in", "$(location {})".format(proto_sync)]
    srcs += [proto_sync]

  if pb3_out:
    cmdparts += ["--pb3-out", "$(RULEDIR)/" + pb3_out]
    outs += [pb3_out]

  if source_patterns:
    cmdparts += ["--source-patterns"] + ['"%s"' % x for x in source_patterns]
  if name_patterns:
    cmdparts += ["--name-patterns"] + ['"%s"' % x for x in name_patterns]

  cmdparts += ["--", "-I ."] + cflags

  native.genrule(
    name = name,
    outs = outs,
    cmd = " ".join(cmdparts),
    srcs = srcs + deps,
    tools = ["//tangent/protostruct:protostruct"],
    visibility = visibility,
  )

  # if proto_out and enforce:
  #   py_test(
  #     name = name + ".enforce",
  #     srcs=["//tangent/protostruct:diff_test.py"],
  #     main="diff_test.py",
  #     args=[
  #       "--truth-file", "$(location %s) " % proto_out,
  #       "--query-file", "$(location %s)" % proto_sync,
  #   ], data = outs + [proto_sync])

  #   py_binary(
  #     name = name + ".fix",
  #     srcs=["//tangent/protostruct:diff_test.py"],
  #     main="diff_test.py",
  #     args=[
  #       "--truth-file", "$(location %s) " % proto_out,
  #       "--query-file", "$(location %s)" % proto_sync,
  #       "--fix"
  #   ], data = outs + [proto_sync])

def protostruct_gen(
    name = None,
    fdset = None,
    basenames = None,
    templates = None,
    visibility = None):
  """Use protostruct to generate code

  Args:
    proto: the .proto file to generate from
    templates: which templates to generate
    visibility: visibility of the generated files
  """
  cmdparts = [
    "$(location //tangent/protostruct:protostruct)",
    "--proto-path",
    ".",
    "generate",
    "--cpp-root",
    "$(BINDIR)",
    "$(location {})".format(fdset),
  ] + templates

  outs = []
  for basename in basenames:
    for groupname in templates:
      if groupname == "cpp-simple":
        outs.append(basename + "-simple.h")
        outs.append(basename + "-simple.cc")
      if groupname == "recon":
        outs.append(basename + "-recon.h")
      if groupname == "pb2c":
        outs.append(basename + ".pb2c.h")
        outs.append(basename + ".pb2c.cc")
      if groupname == "pbwire":
        outs.append(basename + ".pbwire.h")
        outs.append(basename + ".pbwire.c")
      if groupname == "proto":
        outs.append(basename + ".proto")
      if groupname == "cereal":
        outs.append(basename + ".cereal.h")

  native.genrule(
    name = name,
    outs = outs,
    cmd = " ".join(cmdparts),
    srcs = [fdset],
    tools = ["//tangent/protostruct:protostruct"],
    visibility = visibility,
  )
