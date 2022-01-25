def protostruct_gen(
    name = None,
    proto = None,
    templates = None):
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
    "--proto-path", "$$(dirname $<)",
    "generate", "--cpp-root", "$(RULEDIR)",
    proto
  ] + templates

  outs = []
  for groupname in templates:
    if groupname == "cpp-simple":
      outs.append(basename + "-simple.h")
      outs.append(basename + "-simple.cc")
    if groupname == "recon":
      outs.append(basename + "-recon.h")
    if groupname == "pb2c":
      outs.append(basename + "-pb2c.h")
      outs.append(basename + "-pb2c.cc")
    if groupname == "pbwire":
      outs.append(basename + "-pbwire.h")
      outs.append(basename + "-pbwire.c")
    if groupname == "cereal":
      outs.append(basename + "-cereal.h")
      outs.append(basename + "-cereal.c")

  native.genrule(
    name = name,
    outs = outs,
    cmd = " ".join(cmdparts) + " && pwd",
    srcs = [proto],
    tools = ["//tangent/protostruct:protostruct"],
    visibility = ["//visibility:public"],
  )
