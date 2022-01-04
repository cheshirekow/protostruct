#!/usr/bin/env python

import argparse
import collections
import io
import logging
import os
import string
import sys

logger = logging.getLogger(__name__)

TypesInfo = collections.namedtuple(
    "TypesInfo", ["includes", "forward_decls", "get_types"])


def parse_types(typespath):
  includes = []
  forward_decls = []
  get_types = []

  with io.open(typespath, "r", encoding="utf-8") as infile:
    for line in infile:
      line = line.strip()

      # comment or empty line
      if line.startswith("%") or not line.strip():
        continue

      # include line
      if line.startswith("#include"):
        includes.append(line)
        continue

      forward_decls.append('extern GType {}(void);'.format(line))
      get_types.append('  object_types[i++] = {}();'.format(line))

  return TypesInfo(includes, forward_decls, get_types)


def scangobj_run(args, outfile):
  # pylint: disable=import-error
  sys.path.append('/usr/share/gtk-doc/python')
  from gtkdoc import scangobj

  typesinfo = parse_types(args.types)

  outfile.write(scangobj.COMMON_INCLUDES)
  outfile.write("\n".join(typesinfo.includes))
  outfile.write("\n".join(typesinfo.forward_decls))

  if args.query_child_properties:
    outfile.write(
        scangobj.QUERY_CHILD_PROPS_PROTOTYPE & args.query_child_properties)

  # substitute local vars in the template
  type_init_func = args.type_init_func
  main_func_params = "int argc, char *argv[]"
  if "argc" in type_init_func and "argv" not in type_init_func:
    main_func_params = "int argc, G_GNUC_UNUSED char *argv[]"
  elif "argc" not in type_init_func and "argv" in type_init_func:
    main_func_params = "G_GNUC_UNUSED int argc, char *argv[]"
  elif "argc" not in type_init_func and "argv" not in type_init_func:
    main_func_params = "void"

  tplargs = {
      "get_types": "\n".join(typesinfo.get_types),
      "ntypes": len(typesinfo.get_types) + 1,
      "main_func_params": main_func_params,
      "type_init_func": type_init_func,
  }

  names = (
      "signals", "hierarchy", "interfaces", "prerequisites", "args", "actions"
  )
  for name in names:
    tplargs["new_{}_filename".format(name)] = "{}.{}".format(args.module, name)

  main_content = string.Template(scangobj.MAIN_CODE).substitute(tplargs)
  outfile.write(main_content)

  if args.query_child_properties:
    outfile.write(
        scangobj.QUERY_CHILD_PROPS_CODE & args.query_child_properties)

  outfile.write(scangobj.MAIN_CODE_END)


def scangobj_main(args):
  """
  Replicate scangobj.run() with some simplifications and only up through the
  generation of the source code for <module>-scan.c. Don't compile and run
  the program.
  """
  outfile_path = os.path.join(
      args.output_dir, "{}-scan.c".format(args.module))

  with open(outfile_path, "w") as outfile:
    scangobj_run(args, outfile)


def mv_xml(args):
  """
  The behavior of gtkdoc-mkdb has changed and generated xml files are no
  longer put in the xml/ subdirectory. This helper gives us consistent build
  behavior by moving the files to xml/ if needed.

  Given a list of expected output files, if any files expected at xml/foo.xml
  are found at ./foo.xml, they are moved to xml/
  """
  for filepath in args.outfiles:
    pathparts = filepath.split(os.path.sep)
    if not pathparts[-2] == "xml":
      logger.info("Ignoring %s", filepath)
      continue
    filename = pathparts[-1]
    badpath = list(pathparts)
    badpath.pop(-2)
    badpath = os.path.sep.join(badpath)
    if not os.path.exists(filepath) and os.path.exists(badpath):
      logger.info("Moving %s -> xml/%s", badpath, filename)
      os.rename(badpath, filepath)
    else:
      logger.info("Skippig %s", filepath)


def setup_parser(argparser):
  subparsers = argparser.add_subparsers(dest="command")

  parser = subparsers.add_parser("scangobj", help=scangobj_main.__doc__)
  parser.add_argument(
      '--module', required=True,
      help='Name of the doc module being parsed')
  parser.add_argument(
      '--types', default='',
      help='The name of the file to store the types in')
  parser.add_argument(
      '--type-init-func', default='',
      help='The init function(s) to call instead of g_type_init()')
  parser.add_argument(
      '--query-child-properties', default='',
      help='A function that returns a list of child properties for a class')
  parser.add_argument(
      '--output-dir', default='.',
      help='The directory where the results are stored')

  parser = subparsers.add_parser("mv-xml", help=mv_xml.__doc__)
  parser.add_argument(
      'outfiles', nargs="*", help="list of expected output files"
  )


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  setup_parser(parser)
  args = parser.parse_args()

  if args.command == "scangobj":
    return scangobj_main(args)
  if args.command == "mv-xml":
    return mv_xml(args)

  logger.warning("Unrecognized command %s", args.command)
  return 1


if __name__ == "__main__":
  logging.basicConfig(level=logging.INFO)
  sys.exit(main())
