import argparse
import io
import os
import sys
import zipfile

import jinja2
from google import protobuf

from tangent import protostruct

def get_manifest():
  manifest = []

  manifest.append((argparse.__file__, "argparse.py"))
  manifest.append((io.__file__, "io.py"))
  manifest.append((os.__file__, "os.py"))

  for package in (jinja2, protobuf, protostruct):
    directory = "/".join(package.__file__.split("/")[:-1])
    for parent, dirnames, filenames in os.walk(directory):
      reldir = os.path.relpath(parent, directory)
      for dirname in sorted(dirnames):
        if dirname == "__pycache__":
          continue
        manifest.append((
            os.path.join(parent, dirname),
            os.path.join(package.__name__.replace(".", "/"), reldir, dirname)))
      for filename in sorted(filenames):
        if filename.endswith(".pyc"):
          continue
        if filename.endswith(".pyo"):
          continue
        if package is protostruct:
          if filename.endswith(".py") or filename.endswith(".jinja2"):
            pass
          else:
            continue
        manifest.append((
            os.path.join(parent, filename),
            os.path.join(package.__name__.replace(".", "/"), reldir, filename)))
  return manifest

def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument(
      "-o", "--outfile",
      help="path to the output file to create")
  parser.add_argument(
      "-m", "--main",
      help="path to the output file to create")
  args = parser.parse_args()

  with zipfile.ZipFile(args.outfile, "w") as outfile:
    for srcpath, dstpath in get_manifest():
      outfile.write(srcpath, dstpath)
    outfile.writestr("tangent/__init__.py", "")
    outfile.write(args.main, "__main__.py")


if __name__ == "__main__":
  main()
