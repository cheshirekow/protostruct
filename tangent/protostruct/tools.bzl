load("@rules_python//python:defs.bzl", "py_binary", "py_test")

def protostruct_compile(
    name = None,
    proto_path = None,
    sourcefile = None,
    deps = None,
    proto_out = None,
    pb3_out = None,
    source_patterns = None,
    name_patterns = None,
    proto_sync = None,
    visibility = None,
    cflags = None,
    enforce = False):
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

  if not proto_out and not pb3_out:
    fail("One of proto_out or pb3_out are required")

  if proto_path == None:
    proto_path = []
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
    "  --proto-path $$(dirname {})".format(sourcefile),
  ] + ["--proto-path " + path for path in proto_path] + [
    "compile",
    "$(location {})".format(sourcefile),
  ]

  if proto_sync:
    cmdparts += ["  --proto-in", proto_sync]
    srcs += [proto_sync]

  if proto_out:
    cmdparts += ["--proto-out", "$(RULEDIR)/" + proto_out]
    outs += [proto_out]
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

  if proto_out and enforce:
    py_test(
      name = name + ".enforce",
      srcs = ["//tangent/protostruct:diff_test.py"],
      main = "diff_test.py",
      args = [
        "--truth-file",
        "$(location %s) " % proto_out,
        "--query-file",
        "$(location %s)" % proto_sync,
      ],
      data = outs + [proto_sync],
    )

    py_binary(
      name = name + ".fix",
      srcs = ["//tangent/protostruct:diff_test.py"],
      main = "diff_test.py",
      args = [
        "--truth-file",
        "$(location %s) " % proto_out,
        "--query-file",
        "$(location %s)" % proto_sync,
        "--fix",
      ],
      data = outs + [proto_sync],
    )

def protostruct_gen(
    name = None,
    proto = None,
    templates = None,
    visibility = None):
  """Use protostruct to generate code

  Args:
    proto: the .proto file to generate from
    templates: which templates to generate
    visibility: visibility of the generated files
  """
  if proto.endswith(".proto"):
    basename = proto[:-len(".proto")]
  elif proto.endswith(".pb3"):
    basename = proto[:-len(".pb3")]
  else:
    fail("Invalid proto name {}".format(proto))

  cmdparts = [
    "$(location //tangent/protostruct:protostruct)",
    "--proto-path",
    ".",
    "generate",
    "--cpp-root",
    "$(BINDIR)",
    "$(location {})".format(proto),
  ] + templates

  outs = []
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
    if groupname == "cereal":
      outs.append(basename + ".cereal.h")

  native.genrule(
    name = name,
    outs = outs,
    cmd = " ".join(cmdparts),
    srcs = [proto],
    tools = ["//tangent/protostruct:protostruct"],
    visibility = visibility,
  )
