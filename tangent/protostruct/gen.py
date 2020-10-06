"""
Protostruct code generator
"""

import argparse
import io
import os
import sys
import zipfile

import jinja2
from google.protobuf import descriptor_pb2

from tangent import protostruct
from tangent.protostruct import descriptor_extensions_pb2


class ZipfileLoader(jinja2.BaseLoader):
  """
  Jinja template loader capable of loading templates from a zipfile
  """

  def __init__(self, zipfile_path, directory):
    self.zip = zipfile.ZipFile(zipfile_path, mode='r')
    self.dir = directory

  def __del__(self):
    self.zip.close()

  def get_source(self, environment, template):
    # NOTE(josh): not os.path because zipfile uses forward slash
    tplpath = '{}/{}'.format(self.dir, template)
    with self.zip.open(tplpath, 'r') as infile:
      source = infile.read().decode('utf-8')

    return source, tplpath, lambda: True


def get_zipfile_path(modparent):
  """
  If our module is loaded from a zipfile (e.g. a wheel or egg) then return
  the pair (zipfile_path, module_relpath) where zipfile_path is the path to
  the zipfile and module_relpath is the relative path within that zipfile.
  """
  zipfile_parts = modparent.split(os.sep)
  module_parts = []

  while zipfile_parts:
    zipfile_path = os.sep.join(zipfile_parts)
    relative_path = "/".join(module_parts)
    if os.path.exists(zipfile_path) and zipfile.is_zipfile(zipfile_path):
      return zipfile_path, relative_path
    module_parts.insert(0, zipfile_parts.pop(-1))

  return None, None


def setup_argparse(argparser):
  argparser.add_argument("--help", action="help")
  argparser.add_argument(
      "infile",
      help="path to the input file, which is a binary protocol buffer "
      "containing a serialized FileDescriptor")
  argparser.add_argument(
      "--proto-out", "--proto_out",
      help="root of the source tree where to emit proto files")
  argparser.add_argument(
      "--cpp-out", "--cpp_out",
      help="root of the source tree where to emit files")


def get_proto_typename(typeid):
  e = descriptor_pb2.FieldDescriptorProto
  return {
      e.TYPE_BOOL: "bool",
      e.TYPE_BYTES: "bytes",
      e.TYPE_DOUBLE: "double",
      e.TYPE_FIXED32: "fixed32",
      e.TYPE_FIXED64: "fixed64",
      e.TYPE_FLOAT: "float",
      e.TYPE_INT32: "int32",
      e.TYPE_INT64: "int64",
      e.TYPE_SFIXED32: "sfixed32",
      e.TYPE_SFIXED64: "sfixed64",
      e.TYPE_SINT32: "sint32",
      e.TYPE_SINT64: "sint64",
      e.TYPE_STRING: "string",
      e.TYPE_UINT32: "uint32",
      e.TYPE_UINT64: "uint64",
  }[typeid]


def get_wiretype(typeid):
  e = descriptor_pb2.FieldDescriptorProto
  return {
      # Varint
      e.TYPE_INT32: 0,
      e.TYPE_INT64: 0,
      e.TYPE_UINT32: 0,
      e.TYPE_UINT64: 0,
      e.TYPE_SINT32: 0,
      e.TYPE_SINT64: 0,
      e.TYPE_BOOL: 0,
      e.TYPE_ENUM: 0,

      # 64bit
      e.TYPE_FIXED64: 1,
      e.TYPE_SFIXED64: 1,
      e.TYPE_DOUBLE: 1,

      # Length-delmited
      e.TYPE_BYTES: 2,
      e.TYPE_STRING: 2,
      e.TYPE_MESSAGE: 2,

      # 32-bit
      e.TYPE_FIXED32: 5,
      e.TYPE_FLOAT: 5,
      e.TYPE_SFIXED32: 5,
  }[typeid]


def get_tag(fielddescr):
  return (fielddescr.number << 3) | get_wiretype(fielddescr.type)


def get_packed_tag(fielddescr):
  return (fielddescr.number << 3) | 2


def get_enum_columns(enums):
  maxnamelen = 0
  maxnumlen = 0

  for enumdescr in enums:
    maxnamelen = max(maxnamelen, len(enumdescr.name))
    maxnumlen = max(maxnumlen, len("{}".format(enumdescr.number)))

  return "{:%ds} = {:%dd};" % (maxnamelen, maxnumlen)


def get_protostruct_options(descr):
  if not descr.HasField("options"):
    return None

  if isinstance(descr, descriptor_pb2.DescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .ProtostructMessageOptions
        .protostruct_options):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .ProtostructMessageOptions
        .protostruct_options]
  elif isinstance(descr, descriptor_pb2.FieldDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .ProtostructFieldOptions
        .protostruct_options):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .ProtostructFieldOptions
        .protostruct_options]
  elif isinstance(descr, descriptor_pb2.EnumDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .ProtostructEnumOptions
        .protostruct_options):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .ProtostructEnumOptions
        .protostruct_options]
  elif isinstance(descr, descriptor_pb2.EnumValueDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .ProtostructEnumValueOptions
        .protostruct_options):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .ProtostructEnumValueOptions
        .protostruct_options]
  elif isinstance(descr, descriptor_pb2.FileDescriptorProto):
    if not descr.options.HasExtension(
        descriptor_extensions_pb2
        .ProtostructFileOptions
        .protostruct_options):
      return None
    return descr.options.Extensions[
        descriptor_extensions_pb2
        .ProtostructFileOptions
        .protostruct_options]

  return None


def get_comment(descr):
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("protostruct_comment"):
    return ""

  return options.protostruct_comment


def get_lengthfield(descr):
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("length_field"):
    return ""

  return options.length_field


def get_arraysize(descr):
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("array_size"):
    return ""

  return options.array_size


def get_header_filepath(descr):
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("header_filepath"):
    return ""

  return options.header_filepath


def is_repeated(descr):
  """Return true if the field descriptor is for a repeated field"""
  if not descr.HasField("label"):
    return False

  return descr.label == descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED


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


def has_packed_field(descr):
  """Return true if the descriptor contains at least one packed field."""
  for fielddescr in descr.field:
    if is_packed(fielddescr):
      return True
  return False


def has_message_field(descr):
  """Return true if the descriptor contains at least one message field."""
  for fielddescr in descr.field:
    if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
      return True
  return False


def get_labelstr(labelid):
  e = descriptor_pb2.FieldDescriptorProto
  return {
      e.LABEL_REPEATED: "repeated",
      e.LABEL_OPTIONAL: "",
      e.LABEL_REQUIRED: ""
  }[labelid]


def get_label(fielddescr):
  if not fielddescr.HasField("label"):
    return ""
  return get_labelstr(fielddescr.label)


def get_options(fielddescr):
  if not fielddescr.HasField("options"):
    return ""

  options = []
  if fielddescr.options.HasField("packed"):
    if fielddescr.options.packed:
      options.append("packed=true")
    else:
      options.append("packed=false")

  if not options:
    return ""

  # TODO(josh): ugly place to include the padding space
  return " [{}]".format(",".join(options))


def is_primitive(fielddescr):
  if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM:
    return True
  elif fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
    return False
  else:
    return True


def is_message(fielddescr):
  return fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE


def is_enum(fielddescr):
  return fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM


class TemplateContext(object):
  def __init__(self, filedescr):
    self.filedescr = filedescr

  def fqn_typename_cpp(self, descr):
    name_parts = self.filedescr.package.strip(".").split(".")
    name_parts.append(descr.name)
    return "::".join(name_parts)

  def canonicalize_typename(self, typename):
    package_parts = self.filedescr.package.strip(".").split(".")
    typename_parts = typename.strip(".").split(".")

    while (package_parts and typename_parts
           and package_parts[0] == typename_parts[0]):
      package_parts.pop(0)
      typename_parts.pop(0)

    return ".".join(typename_parts)

  def get_typename(self, fielddescr):
    if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM:
      return self.canonicalize_typename(fielddescr.type_name)
    elif fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
      return self.canonicalize_typename(fielddescr.type_name)
    else:
      return get_proto_typename(fielddescr.type)

  def get_emit_fun(self, fielddescr):
    """Return the name of the emit function for a single value of the given
       type."""

    if is_primitive(fielddescr):
      return "pbemit_" + self.get_typename(fielddescr)

    return "_pbemit_" + self.get_typename(fielddescr)

  def tuplize_fielddescr(self, fielddescr):
    return (
        get_label(fielddescr),
        self.get_typename(fielddescr),
        fielddescr.name,
        fielddescr.number,
        get_options(fielddescr),
        get_comment(fielddescr))

  def get_field_columns(self, fields):
    lengths = [0] * 6

    for fielddescr in fields:
      lengths = [
          max(maxlen, len("{}".format(item))) for maxlen, item
          in zip(lengths, self.tuplize_fielddescr(fielddescr))]

    format_parts = []

    # label, e.g. "repeated"
    if lengths[0]:
      format_parts.append("{:%ds} " % lengths[0])
    else:
      format_parts.append("{}")

    # <type> <name> = <number>
    format_parts.append("{:>%ds} {:%ds} = {:%dd}" % tuple(lengths[1:4]))

    # options, e.g. [packed=true]
    format_parts.append("{}")
    format_parts.append(";")

    if lengths[5]:
      format_parts.append(" {}")
    else:
      format_parts.append("{}")

    return "".join(format_parts)


def format_reserved(ranges):
  out = []
  for rrange in ranges:
    if rrange.end is None:
      out.append("{} to max".format(rrange.start))
    elif rrange.end == rrange.start + 1:
      out.append("{}".format(rrange.start))
    else:
      out.append("{} to {}".format(rrange.start, rrange.end - 1))
  return ", ".join(out)


def main():
  argparser = argparse.ArgumentParser(description=__doc__, add_help=False)
  setup_argparse(argparser)
  try:
    import argcomplete
    argcomplete.autocomplete(argparser)
  except ImportError:
    pass

  args = argparser.parse_args()

  filedescr = descriptor_pb2.FileDescriptorProto()
  with io.open(args.infile, "rb") as infile:
    filedescr.ParseFromString(infile.read())

  zipfile_path, package_path = get_zipfile_path(
      os.path.dirname(protostruct.__file__))
  if zipfile_path:
    jenv = jinja2.Environment(
        loader=ZipfileLoader(zipfile_path,
                             package_path + '/templates'))
  else:
    thisdir = os.path.dirname(__file__)
    tpldir = os.path.join(thisdir, "templates")
    jenv = jinja2.Environment(loader=jinja2.FileSystemLoader(tpldir))

  ctx = TemplateContext(filedescr)
  jenv.globals.update(
      format_reserved=format_reserved,
      fqn_typename_cpp=ctx.fqn_typename_cpp,
      get_arraysize=get_arraysize,
      get_comment=get_comment,
      get_emit_fun=ctx.get_emit_fun,
      get_enum_columns=get_enum_columns,
      get_header_filepath=get_header_filepath,
      get_field_columns=ctx.get_field_columns,
      get_label=get_label,
      get_lengthfield=get_lengthfield,
      get_packed_tag=get_packed_tag,
      get_tag=get_tag,
      get_typename=ctx.get_typename,
      has_message_field=has_message_field,
      has_packed_field=has_packed_field,
      is_enum=is_enum,
      is_message=is_message,
      is_packed=is_packed,
      is_primitive=is_primitive,
      is_repeated=is_repeated,
      tuplize_fielddescr=ctx.tuplize_fielddescr,
      LABEL_REPEATED=descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED,
      TYPE_MESSAGE=descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE,
  )

  gensource = get_header_filepath(filedescr)
  relpath_outdir = "/".join(gensource.split("/")[:-1])
  basename = gensource.split("/")[-1].split(".")[0]

  process_pairs = []
  for suffix in [".proto", ".pbwire.h", ".pbwire.c", ".pb2c.h", ".pb2c.cc"]:
    outfile_name = basename + suffix
    if suffix.endswith(".proto"):
      outfile_dir = os.path.join(args.proto_out, relpath_outdir)
    else:
      outfile_dir = os.path.join(args.cpp_out, relpath_outdir)

    if not os.path.exists(outfile_dir):
      os.makedirs(outfile_dir)

    outfile_path = os.path.join(outfile_dir, outfile_name)
    template_name = "XXX" + suffix + ".jinja2"
    process_pairs.append((outfile_path, template_name))

  if relpath_outdir:
    include_base = os.path.join(relpath_outdir, basename)
  else:
    include_base = basename

  for outpath, template_name in process_pairs:
    if not outpath:
      continue

    template = jenv.get_template(template_name)
    content = template.render(
        filedescr=filedescr,
        include_base=include_base)
    if outpath == "-":
      outpath = os.dup(1)
    with io.open(outpath, "w", encoding="utf-8") as outfile:
      outfile.write(content)
      outfile.write("\n")


if __name__ == "__main__":
  sys.exit(main())
