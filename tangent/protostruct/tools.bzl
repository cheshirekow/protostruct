def protostruct_gen(
    proto = None,
    only = None):
  """Use protostruct to generate code

  Args:
    proto: the .proto file to generate from
    only: limit protostruct to generating only the specified output groups
  """
  if not proto.endswith(".proto"):
    fail("Invalid proto name {}".format(proto))
  basename = proto[:-len(".proto")]

  cmdparts = [
    "$(location //tangent/protostruct:protostruct)",
    proto,
    "--proto-path",
    "$$(dirname $<)",
    "--outfile",
    "$(RULEDIR)/{}.pb3".format(basename),
    "--cpp-out",
    "$(RULEDIR)",
  ]

  outs = []
  if only:
    cmdparts.append("--only")
  for groupname in only:
    cmdparts.append(groupname)
    if groupname == "cpp-simple":
      outs.append(basename + "-simple.h")
      outs.append(basename + "-simple.cc")

  native.genrule(
    name = "protostruct-gen-" + basename + ".h",
    outs = outs,
    cmd = " ".join(cmdparts),
    srcs = [proto],
    tools = ["//tangent/protostruct:protostruct"],
    visibility = ["//visibility:public"],
  )
