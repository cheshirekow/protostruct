"""
Stateless utility methods. This module provides utility functions to the
jinja template environment.
"""

from google.protobuf import descriptor_pb2

from tangent.protostruct import descriptor_extensions_pb2


def format_reserved(ranges):
  """Given a list of reserved fieldnumber ranges, format a `reserved`
  declaration for emission in .proto. For example, given the ranges::

    [(1,3), (4,5), (6,None)]

  , this function will return the string::

  "1 to 2, 4, 6 to max"

  ."""
  out = []
  for rrange in ranges:
    if rrange.end is None:
      out.append("{} to max".format(rrange.start))
    elif rrange.end == rrange.start + 1:
      out.append("{}".format(rrange.start))
    else:
      out.append("{} to {}".format(rrange.start, rrange.end - 1))
  return ", ".join(out)


def get_arraysize(descr):
  """Given a FieldDescriptor for a repeated message, return the capacity
    of the fixed sized C-array genrated in the C-bindings. The capacity
    may be a string (e.g. the name of a macro or enum) or an integer."""
  options = get_protostruct_options(descr)
  if options is None:
    raise ValueError("Missing options extension for %s" % descr)

  if not options.HasField("capacity"):
    raise ValueError("Missing capacity option for %s -- %s" % (descr, options))

  if options.capname:
    return options.capname

  return options.capacity


def get_enum_columns(enums, style=None):
  """Return a format string used for each enumerator value declaration in a
     a generated code file (where the language is indicated by `style`).
     The format string is something like '{:6s} = {:12s};' but the
     field widths are computed by scanning all the values and finding the
     largest string, such that the output is columnized."""

  if style is None:
    style = "proto"

  if enums:
    maxnamelen = max(len(enumdescr.name) for enumdescr in enums)
    maxnumlen = max(len("{}".format(enumdescr.number)) for enumdescr in enums)
  else:
    maxnamelen = 8
    maxnumlen = 3

  style = style.lower()
  if style == "proto":
    return "{:%ds} = {:%dd}" % (maxnamelen, maxnumlen)
  if style in ("c", "cpp"):
    return "{:%ds} = {:%dd}" % (maxnamelen, maxnumlen)
  raise ValueError("Unexpected style: {}".format(style))


def get_header_filepath(descr):
  """Given a FileDescriptorProto which was reverse compiled from a C header
     file, return the filepath of the the header which was processed."""
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("header_filepath"):
    return ""

  return options.header_filepath


def get_label(fielddescr):
  """
  Given a field descriptor, return any label string used (e.g. "repeated")
  to declare that field in .proto.
  """
  if not fielddescr.HasField("label"):
    return ""
  return get_labelstr(fielddescr.label)


def get_labelstr(labelid):
  """Given the id of a label from a FieldDescriptorProto, return the string
     used to indicate that label in proto3 .proto files."""
  proto = descriptor_pb2.FieldDescriptorProto
  return {
      proto.LABEL_REPEATED: "repeated",
      proto.LABEL_OPTIONAL: "",
      proto.LABEL_REQUIRED: ""
  }[labelid]


def get_lengthfield(descr):
  """Given a FieldDescriptor for a repeated message, return the name of the
     field generated in the C-bindings to hold the occupied size of the
     repeated field."""
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("lenfield"):
    return ""

  return options.lenfield


def get_packed_tag(fielddescr):
  """Return the tag number for a packed repeated field"""
  return (fielddescr.number << 3) | 2


def get_protostruct_options(descr):
  """Return the extension object for the given descriptor."""

  # pylint: disable=too-many-return-statements
  if not descr.HasField("options"):
    return None

  if isinstance(descr, descriptor_pb2.DescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .msgopts):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .msgopts]
  if isinstance(descr, descriptor_pb2.FieldDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .fieldopts):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .fieldopts]
  if isinstance(descr, descriptor_pb2.EnumDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .enumopts):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .enumopts]
  if isinstance(descr, descriptor_pb2.EnumValueDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .enumvopts):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .enumvopts]
  if isinstance(descr, descriptor_pb2.FileDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .fileopts):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .fileopts]

  return None


def get_tag(fielddescr):
  """Return an integer "tag" which is a bitfield composed of the fieldid
     and the wire type."""
  return (fielddescr.number << 3) | get_wiretype(fielddescr.type)


def get_wiretype(typeid):
  """Given a FileDescriptorProto.Type enumeration, return the protobuf
     wire type that will be written to the serialized representation. """
  proto = descriptor_pb2.FieldDescriptorProto
  return {
      # Varint
      proto.TYPE_INT32: 0,
      proto.TYPE_INT64: 0,
      proto.TYPE_UINT32: 0,
      proto.TYPE_UINT64: 0,
      proto.TYPE_SINT32: 0,
      proto.TYPE_SINT64: 0,
      proto.TYPE_BOOL: 0,
      proto.TYPE_ENUM: 0,

      # 64bit
      proto.TYPE_FIXED64: 1,
      proto.TYPE_SFIXED64: 1,
      proto.TYPE_DOUBLE: 1,

      # Length-delmited
      proto.TYPE_BYTES: 2,
      proto.TYPE_STRING: 2,
      proto.TYPE_MESSAGE: 2,

      # 32-bit
      proto.TYPE_FIXED32: 5,
      proto.TYPE_FLOAT: 5,
      proto.TYPE_SFIXED32: 5,
  }[typeid]


def has_message_field(descr):
  """Return true if the descriptor contains at least one message field."""
  for fielddescr in descr.field:
    if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
      return True
  return False


def has_packed_field(descr):
  """Return true if the descriptor contains at least one packed field."""
  for fielddescr in descr.field:
    if is_packed(fielddescr):
      return True
  return False


def is_enum(fielddescr):
  return fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM


def is_message(fielddescr):
  return fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE


def is_packed(descr):
  """Return true if the fielddescr is for a packed, repeated, primitive field
  """
  if not is_repeated(descr):
    return False

  if not descr.HasField("options"):
    return False

  if not descr.options.HasField("packed"):
    return False

  return descr.options.packed


def is_primitive(fielddescr):
  if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM:
    return True

  if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
    return False

  return True


def is_repeated(descr):
  """Return true if the field descriptor is for a repeated field"""
  if not descr.HasField("label"):
    return False

  return descr.label == descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED
