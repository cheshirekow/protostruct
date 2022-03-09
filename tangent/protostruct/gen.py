# PYTHON_ARGCOMPLETE_OK
"""
Simple protobuf code generation frontend using Jinja as a template engine.

Read a serialized FileDescriptorSet from disk and render a jinja template
once for each FileDescriptor in the set.
"""

import argparse
import io
import logging
import os
import zipfile

import jinja2
from google.protobuf import descriptor_pb2

from tangent import protostruct
from tangent.protostruct import template_util as util
from tangent.protostruct.template_context import TemplateContext

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
      "--descriptor-set-in",
      help="path to the input file, which is a binary protocol buffer "
      "containing a serialized FileDescriptor")
  argparser.add_argument(
      "--proto-out", "--proto_out",
      help="root of the source tree where to emit .proto files")
  argparser.add_argument(
      "--cpp-root", "--cpp_root",
      help="root of the source tree where to emit C++ files")
  argparser.add_argument(
      "templates", nargs="*",
      help="Only generate a specific set of outputs")


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

  fileset = descriptor_pb2.FileDescriptorSet()
  with io.open(args.descriptor_set_in, "rb") as infile:
    fileset.ParseFromString(infile.read())

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

  for filedescr in fileset.file:
    jenv.globals.update(
        enumerate=enumerate,
        ctx=TemplateContext(filedescr),
        util=util,
        LABEL_REPEATED=descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED,
        TYPE_MESSAGE=descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE,
    )

    gensource = util.get_header_filepath(filedescr)
    if not gensource and filedescr.name:
      gensource = filedescr.name
    relpath_outdir = "/".join(gensource.split("/")[:-1])

    if not gensource:
      logger.fatal(
          "No basename specified and no filepath in the FileDescriptor proto")
    basename = gensource.split("/")[-1].split(".")[0]

    suffixes = []
    for group in args.templates:
      suffixes.extend(TEMPLATES[group])

    process_pairs = []
    for suffix in suffixes:
      outfile_name = basename + suffix
      outfile_dir = os.path.join(args.cpp_root, relpath_outdir)

      if not os.path.exists(outfile_dir):
        os.makedirs(outfile_dir)

      outfile_path = os.path.join(outfile_dir, outfile_name)
      template_name = "XXX" + suffix + ".jinja2"
      process_pairs.append((outfile_path, template_name))

    if relpath_outdir:
      include_base = os.path.join(relpath_outdir, basename)
    else:
      include_base = basename

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
      logger.info("Wrote %s", outpath)


if __name__ == "__main__":
  # NOTE(josh): don't sys.exit or the interpreter will kill the process and
  # Py_Main wont return, so we wont cleanup tempfiles in the main program
  # (if needed).
  logging.basicConfig()
  main()
