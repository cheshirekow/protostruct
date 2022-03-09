"""
Statefule utility methods. This module provides utility functions to the
jinja template environment.
"""

import json
import re

from google.protobuf import descriptor_pb2
from google.protobuf import text_format

from tangent.protostruct import template_util as util


def get_proto_typename(typeid):
  """Given a FileDescriptorProto.Type enumeration, return the string
     representation of that type used in a .proto file"""
  proto = descriptor_pb2.FieldDescriptorProto
  return {
      proto.TYPE_BOOL: "bool",
      proto.TYPE_BYTES: "bytes",
      proto.TYPE_DOUBLE: "double",
      proto.TYPE_FIXED32: "fixed32",
      proto.TYPE_FIXED64: "fixed64",
      proto.TYPE_FLOAT: "float",
      proto.TYPE_INT32: "int32",
      proto.TYPE_INT64: "int64",
      proto.TYPE_SFIXED32: "sfixed32",
      proto.TYPE_SFIXED64: "sfixed64",
      proto.TYPE_SINT32: "sint32",
      proto.TYPE_SINT64: "sint64",
      proto.TYPE_STRING: "string",
      proto.TYPE_UINT32: "uint32",
      proto.TYPE_UINT64: "uint64",
  }[typeid]


def get_simple_cpp_typename(typeid):
  """Given a FileDescriptorProto.Type enumeration, return the corresponding
     type used by the simple-cpp template. """
  proto = descriptor_pb2.FieldDescriptorProto
  return {
      proto.TYPE_BOOL: "bool",
      proto.TYPE_BYTES: "std::vector<uint8_t>",
      proto.TYPE_DOUBLE: "double",
      proto.TYPE_FIXED32: "int32_t",
      proto.TYPE_FIXED64: "int64_t",
      proto.TYPE_FLOAT: "float",
      proto.TYPE_INT32: "int32_t",
      proto.TYPE_INT64: "int64_t",
      proto.TYPE_SFIXED32: "int32_t",
      proto.TYPE_SFIXED64: "int32_t",
      proto.TYPE_SINT32: "int32_t",
      proto.TYPE_SINT64: "int64_t",
      proto.TYPE_STRING: "std::string",
      proto.TYPE_UINT32: "uint32_t",
      proto.TYPE_UINT64: "uint64_t",
  }[typeid]


def get_options(fielddescr):
  if not fielddescr.HasField("options"):
    return ""

  options = []
  if fielddescr.options.HasField("packed"):
    if fielddescr.options.packed:
      options.append("packed=true")
    else:
      options.append("packed=false")

  psopts = util.get_protostruct_options(fielddescr)
  if psopts:
    opsdict = {}
    if psopts.HasField("fieldtype"):
      opsdict["fieldtype"] = psopts.fieldtype
    if psopts.lenfield:
      # We only need to record this if it's not the default
      # TODO(josh): make a configuration option
      if psopts.lenfield != fielddescr.name + "Count":
        opsdict["lenfield"] = psopts.lenfield
    if psopts.capacity:
      opsdict["capacity"] = psopts.capacity
    if psopts.capname:
      opsdict.pop("capacity", None)
      opsdict["capname"] = psopts.capname

    if len(opsdict) > 1:
      options.append(
          "(protostruct.fieldopts) = {%s}"
          % text_format.MessageToString(psopts, as_one_line=True))
    elif opsdict:
      for key, value in opsdict.items():
        options.append(
            "(protostruct.fieldopts).{}={}".format(key, json.dumps(value)))

  if not options:
    return ""

  # TODO(josh): ugly place to include the padding space
  return " [{}]".format(", ".join(options))


def format_leading_comment(commentstr, style):
  lines = commentstr.strip().split("\n")
  if style == "cpp":
    return "/// " + "\n/// ".join(lines)
  raise ValueError("Unknown style {}".format(style))


def format_trailing_comment(commentstr, style):
  lines = commentstr.strip().split("\n")
  if style == "cpp":
    return "//!< " + "\n//!< ".join(lines)
  raise ValueError("Unknown style {}".format(style))


class TemplateContext(object):
  """
  Stateful utility methods. This object is provided to the jinja template
  environment and provides utility functions used by the templates. Methods
  of this object are utility functions that require access to the active
  FileDescriptorProto for the template render.
  """

  def __init__(self, filedescr):
    self.filedescr = filedescr

  def get_cpp_namespace(self):
    """
    Convert the proto package into a C++ qualified namespace string. For
    example, if the source .proto had::

      package foo.bar

    then this function returns "foo::bar".
    """
    name_parts = self.filedescr.package.strip(".").split(".")
    return "::".join(name_parts)

  def fqn_typename_cpp(self, descr):
    """
    Given an EnumDescriptorProto or DescriptorProto taken from this
    FileDescriptorProto, return the fully-qualified C++ name. For
    example, if the source .proto had::

      package foo.bar
      message Baz {}

    then, given the DescriptorProto for `Baz`, this function would
    return "foo::bar::Baz".
    """
    name_parts = self.filedescr.package.strip(".").split(".")
    name_parts.append(descr.name)
    return "::".join(name_parts)

  def canonicalize_typename(self, typename, style=None):
    """
    Given a qualified protobuf typename for a message or enum, strip the
    package qualifications that are redundant due to the fact that the
    package of the active FileDescriptorProto shares the same package prefix.

    For example, if the source .proto had::

      package foo.bar

    then, given the `typename` "foo.bar.Baz`, this function would return
    "Baz", because "foo.bar" is redundant due to the fact that the current
    FileDescriptorProto is already in package `foo.bar`.
    """
    if style is None:
      style = "proto"

    package_parts = self.filedescr.package.strip(".").split(".")
    typename_parts = typename.strip(".").split(".")

    while (package_parts and typename_parts
           and package_parts[0] == typename_parts[0]):
      package_parts.pop(0)
      typename_parts.pop(0)

    if style == "proto":
      return ".".join(typename_parts)
    if style == "cpp":
      return "::".join(typename_parts)
    raise ValueError("Unknown style: %s" % style)

  def get_typename(self, fielddescr, style=None):
    """Return a string containing the type of the field described in the
       FieldDescriptorProto. """

    if style is None:
      style = "proto"

    if (not fielddescr.HasField("type")
        or fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE):
      return self.canonicalize_typename(fielddescr.type_name, style)

    if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM:
      return self.canonicalize_typename(fielddescr.type_name, style)

    if style == "proto":
      return get_proto_typename(fielddescr.type)
    if style == "cpp":
      psopts = util.get_protostruct_options(fielddescr)
      if psopts and psopts.HasField("fieldtype"):
        return psopts.fieldtype
      return get_simple_cpp_typename(fielddescr.type)

    raise ValueError("Unknown style %s" % style)

  def get_pbparse(self, fielddescr):
    """Return the name of the pbparse function for a given field descriptor.
       The pbparse function will depend on both the proto wire format and
       the C field type. """

    popts = util.get_protostruct_options(fielddescr)
    if popts and popts.HasField("fieldtype"):
      return "pbparse_" + re.sub("(.*)(?:_t)", r"\1", popts.fieldtype)

    return "pbparse_" + self.get_typename(fielddescr)

  def get_emit_fun(self, fielddescr, passno=None):
    """Return the name of the emit function for a single value of the given
       type."""

    if util.is_primitive(fielddescr):
      return "pbemit_" + self.get_typename(fielddescr)

    return "_pbemit{}_".format(passno) + self.get_typename(fielddescr)

  def get_sourcecodeinfo_location(self, query_path):
    if not self.filedescr.HasField("source_code_info"):
      return None

    for location in self.filedescr.source_code_info.location:
      if location.path == query_path:
        return location
    return None

  def get_leading_comment(self, query_path, style):
    """Lookup a comment py source code location path within the
       filedescriptor."""
    location = self.get_sourcecodeinfo_location(query_path)
    if location is None:
      return ""

    if location.leading_comments:
      return format_leading_comment(location.leading_comments, style)
    return ""

  def get_trailing_comment(self, query_path, style):
    """Lookup a comment py source code location path within the
       filedescriptor."""
    location = self.get_sourcecodeinfo_location(query_path)
    if location is None:
      return ""

    if location.trailing_comments:
      return format_trailing_comment(location.trailing_comments, style)
    return ""

  def get_comment(self, descr):
    """Lookup the protostruct extension fields, if the exist, and retrieve
      any comments from them."""
    options = util.get_protostruct_options(descr)
    if options is None:
      return ""

    if not options.HasField("comment"):
      return ""

    return options.comment

  def tuplize_fielddescr(self, fielddescr, style=None):
    """Given a FieldDescriptorProto return a tuple containing the parts of
       a field declaration in either .proto or C++ format. In .proto format
       the tuple contains (label, typename, name, number, options, comment)
       and in C++ format it contains (typename, name, comment)."""
    if style is None:
      style = "proto"
    if style == "proto":
      return (
          util.get_label(fielddescr),
          self.get_typename(fielddescr, style),
          fielddescr.name,
          fielddescr.number,
          get_options(fielddescr),
          self.get_comment(fielddescr))
    if style == "cpp":
      return (
          self.get_typename(fielddescr, style),
          fielddescr.name,
          self.get_comment(fielddescr))
    raise ValueError("Unknown style: %s" % style)

  def accumulate_fieldlengths(self, fields, style):
    """Go through each field descriptor and format each item in the
    descriptor as a string. Get the length of that string, and accumulate
    the max of these lengths over all descriptors. Return a list of these
    accumulated lengths. `out[idx]` contains the length of the largest
    string for item `idx` in the field descriptors."""
    lengths = [0] * 10

    for fielddescr in fields:
      lengths = [
          max(maxlen, len("{}".format(item))) for maxlen, item
          in zip(lengths, self.tuplize_fielddescr(fielddescr, style))]

    return lengths

  def get_field_columns(self, fields, style=None):
    """Return a format string that looks something like
    `"{:8d} {:>12s} {:6s} = {:3d};"` suitable for formatting a proto
    field declaration like `"repeated FooMessage foo_field =  12;"` with
    fixed columns.
    """
    if style is None:
      style = "proto"

    lengths = self.accumulate_fieldlengths(fields, style)
    format_parts = []

    if style == "proto":
      # label, e.g. "repeated"
      if lengths[0]:
        format_parts.append("{:%ds} " % lengths[0])
      else:
        format_parts.append("{}")

      # <type> <name> = <number>
      format_parts.append("{:>%ds} {:%ds} = {:%dd}" % tuple(lengths[1:4]))

      # options, e.g. [packed=true]
      format_parts.append("{}")

    elif style == "cpp":
      # <type> <name>;
      format_parts.append("{:>%ds} {:%ds}" % tuple(lengths[0:2]))

    else:
      raise ValueError("Unknown style: %s" % style)

    format_parts.append(";")

    # comments
    if lengths[-1]:
      format_parts.append("  {}")
    else:
      format_parts.append("{}")

    return "".join(format_parts)
