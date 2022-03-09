"""
Create a tarball containing the compiled protostruct binary and all the
non-standard .so's that it depends on so that it can be distributed into
another bazel build.
"""

import argparse
import io
import logging
import os
import re
import subprocess
import tarfile

logger = logging.getLogger(__name__)


def iswellknown(filname):
  for pattern in WELL_KNOWN_SOs:
    if pattern.match(filename):
      return True
  return False


logging.basicConfig(level=logging.INFO)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--protostruct-path")
parser.add_argument("--protostruct-header-path")
parser.add_argument("--tools-bzl-path")
parser.add_argument("--outfile")
args = parser.parse_args()

WELL_KNOWN_SOs = [re.compile(x) for x in [
  r"linux-vdso\.so.*",
  r"libz\.so.*",
  r"libm\.so.*",
  r"libstdc\+\+\.so.*",
  r"libc\.so.*",
  r".*/ld-linux-.*\.so.*",
  r"libunwind.*",
  r"libgcc_s.so.*",
  r"libpthread.so.*",
]]

deps = []
buildcontent = ["""
load(
  "@rules_cc//cc:defs.bzl",
  "cc_binary",
  "cc_import",
)
"""]

dlmap = {}
for cwd, dirnames, filenames in os.walk(args.protostruct_header_path + ".runfiles"):
  dirnames[:] = sorted(dirnames)
  for filename in sorted(filenames):
    dlmap[filename] = os.path.join(cwd, filename)

with tarfile.open(args.outfile, "w:gz", dereference=True) as outfile:
  outfile.add(args.protostruct_path, "protostruct.elf")

  lddcontent = subprocess.check_output(["ldd", args.protostruct_header_path]).decode("utf-8")
  for line in lddcontent.split("\n"):
    if "=>" not in line:
      continue
    filename, foundpath = (x.strip() for x in line.split("=>"))
    if iswellknown(filename):
      continue
    if filename not in dlmap:
      logger.info("Missing runfile for %s", filename)
      continue

    logger.info("Adding %s -> %s", filename, dlmap[filename])
    outfile.add(dlmap[filename], filename)

    name = filename.split(".")[0]
    deps.append(name)
    buildcontent.append("""
cc_import(
  name = "{name}",
  shared_library = "{filename}",
  visibility = ["//visibility:public"],
)
    """.format(filename=filename, name=name))

  buildcontent.append("""
sh_binary(
  name="protostruct",
  srcs=["protostruct.elf"],
  data=[{deps}],
  visibility = ["//visibility:public"],
)
  """.format(deps=",\n    ".join(['":{}"'.format(x) for x in deps])))

  data = "\n".join(buildcontent).encode("utf-8")
  tarinfo = tarfile.TarInfo('BUILD')
  tarinfo.size = len(data)
  outfile.addfile(tarinfo, io.BytesIO(data))

  with open(args.tools_bzl_path) as infile:
    content = infile.read()
  data = content.replace("//tangent/protostruct", "@protostruct//").encode("utf-8")
  tarinfo = tarfile.TarInfo('tools.bzl')
  tarinfo.size = len(data)
  outfile.addfile(tarinfo, io.BytesIO(data))
