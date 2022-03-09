"""
Protostruct code generator
"""

import argparse
import io
import json
import logging
import os
import zipfile

import jinja2
from google.protobuf import descriptor_pb2
from google.protobuf import text_format

from tangent import protostruct
from tangent.protostruct import descriptor_extensions_pb2

logger = logging.getLogger(__name__)


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
  argparser.add_argument(
      "infile",
      help="path to the input file, which is a binary protocol buffer "
      "containing a serialized FileDescriptor")
  argparser.add_argument(
      "--basename",
      help="if provided, use this basename, instead of the basename of infile")
  argparser.add_argument(
      "--proto-out", "--proto_out",
      help="path to write .proto, if desired")
  argparser.add_argument(
      "--cpp-root", "--cpp_root",
      help="root of the source tree where to emit files")
  argparser.add_argument(
      "templates", nargs="*",
      help="Only generate a specific set of outputs")


def get_proto_typename(typeid):
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


def get_wiretype(typeid):
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


def get_tag(fielddescr):
  return (fielddescr.number << 3) | get_wiretype(fielddescr.type)


def get_packed_tag(fielddescr):
  return (fielddescr.number << 3) | 2


def get_enum_columns(enums, style=None):
  """Return a format string used for each enum value declaration in a .proto
     file. The format string is something like '{:6s} = {:12s};' but the
     field widths are computed according to ."""

  if style is None:
    style = "proto"

  maxnamelen = max(len(enumdescr.name) for enumdescr in enums)
  maxnumlen = max(len("{}".format(enumdescr.number)) for enumdescr in enums)

  style = style.lower()
  if style == "proto":
    return "{:%ds} = {:%dd}" % (maxnamelen, maxnumlen)
  if style in ("c", "cpp"):
    return "{:%ds} = {:%dd}" % (maxnamelen, maxnumlen)
  raise ValueError("Unexpected style: {}".format(style))


def get_protostruct_options(descr):
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


def get_lengthfield(descr):
  options = get_protostruct_options(descr)
  if options is None:
    return ""

  if not options.HasField("lenfield"):
    return ""

  return options.lenfield


def get_arraysize(descr):
  options = get_protostruct_options(descr)
  if options is None:
    raise ValueError("Missing options extension for %s" % descr)

  if not options.HasField("capacity"):
    raise ValueError("Missing capacity option for %s -- %s" % (descr, options))

  if options.capname:
    return options.capname

  return options.capacity


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
  proto = descriptor_pb2.FieldDescriptorProto
  return {
      proto.LABEL_REPEATED: "repeated",
      proto.LABEL_OPTIONAL: "",
      proto.LABEL_REQUIRED: ""
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

  psopts = get_protostruct_options(fielddescr)
  if psopts:
    opsdict = {}
    if psopts.HasField("fieldtype"):
      psopts.ClearField("fieldtype")
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


def is_primitive(fielddescr):
  if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM:
    return True

  if fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
    return False

  return True


def is_message(fielddescr):
  return fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE


def is_enum(fielddescr):
  return fielddescr.type == descriptor_pb2.FieldDescriptorProto.TYPE_ENUM


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
  def __init__(self, filedescr):
    self.filedescr = filedescr

  def get_cpp_namespace(self):
    name_parts = self.filedescr.package.strip(".").split(".")
    return "::".join(name_parts)

  def fqn_typename_cpp(self, descr):
    name_parts = self.filedescr.package.strip(".").split(".")
    name_parts.append(descr.name)
    return "::".join(name_parts)

  def canonicalize_typename(self, typename, style=None):
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
      return get_simple_cpp_typename(fielddescr.type)

    raise ValueError("Unknown style %s" % style)

  def get_emit_fun(self, fielddescr, passno=None):
    """Return the name of the emit function for a single value of the given
       type."""

    if is_primitive(fielddescr):
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
    options = get_protostruct_options(descr)
    if options is None:
      return ""

    if not options.HasField("comment"):
      return ""

    return options.comment

  def tuplize_fielddescr(self, fielddescr, style=None):
    if style is None:
      style = "proto"
    if style == "proto":
      return (
          get_label(fielddescr),
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
    """Go through each field descriptor and get format each item in the
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


# Map a named group of functionality to a set of templates that implement
# that functionality
TEMPLATES = {
    "proto": [".proto"],
    "cereal": [".cereal.h"],
    "pbwire": [".pbwire.h", ".pbwire.c"],
    "pb2c": [".pb2c.h", ".pb2c.cc"],
    "cpp-simple": ["-simple.h", "-simple.cc"],
    "recon": ["-recon.h"]
}


def main():
  argparser = argparse.ArgumentParser(description=__doc__)
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
        trim_blocks=True, lstrip_blocks=True,
        loader=ZipfileLoader(zipfile_path,
                             package_path + '/templates'))
  else:
    thisdir = os.path.dirname(__file__)
    tpldir = os.path.join(thisdir, "templates")
    jenv = jinja2.Environment(loader=jinja2.FileSystemLoader(tpldir))

  ctx = TemplateContext(filedescr)
  jenv.globals.update(
      enumerate=enumerate,
      ctx=ctx,
      format_reserved=format_reserved,
      fqn_typename_cpp=ctx.fqn_typename_cpp,
      get_arraysize=get_arraysize,
      get_comment=ctx.get_comment,
      get_cpp_namespace=ctx.get_cpp_namespace,
      get_emit_fun=ctx.get_emit_fun,
      get_enum_columns=get_enum_columns,
      get_field_columns=ctx.get_field_columns,
      get_header_filepath=get_header_filepath,
      get_label=get_label,
      get_lengthfield=get_lengthfield,
      get_packed_tag=get_packed_tag,
      get_protostruct_options=get_protostruct_options,
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

  if not args.basename:
    args.basename = gensource.split("/")[-1].split(".")[0]

  suffixes = []
  for group in args.templates:
    suffixes.extend(TEMPLATES[group])

  process_pairs = []
  for suffix in suffixes:
    outfile_name = args.basename + suffix
    outfile_dir = os.path.join(args.cpp_root, relpath_outdir)

    if not os.path.exists(outfile_dir):
      os.makedirs(outfile_dir)

    outfile_path = os.path.join(outfile_dir, outfile_name)
    template_name = "XXX" + suffix + ".jinja2"
    process_pairs.append((outfile_path, template_name))

  if relpath_outdir:
    include_base = os.path.join(relpath_outdir, args.basename)
  else:
    include_base = args.basename

  if args.proto_out:
    process_pairs.append((args.proto_out, "XXX.proto.jinja2"))

  for outpath, template_name in process_pairs:
    if not outpath:
      continue

    template = jenv.get_template(template_name)
    content = template.render(
        filedescr=filedescr,
        include_base=include_base)
    if outpath == "-":
      outpath = os.dup(1)
    outdir = os.path.dirname(outpath)
    if not os.path.exists(outdir):
      os.makedirs(outdir)
    with io.open(outpath, "w", encoding="utf-8") as outfile:
      outfile.write(content)
      outfile.write("\n")


if __name__ == "__main__":
  # NOTE(josh): don't sys.exit or the interpreter will kill the process and
  # Py_Main wont return, so we wont cleanup tempfiles in the main program
  # (if needed).
  main()
