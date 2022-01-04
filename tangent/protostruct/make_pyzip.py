import argparse
import io
import os
import zipfile

from google import protobuf
import jinja2
import markupsafe
import six

from tangent import protostruct


def get_manifest(bindir):
  manifest = []

  manifest.append((argparse.__file__, "argparse.py"))
  manifest.append((io.__file__, "io.py"))
  manifest.append((os.__file__, "os.py"))

  for package in (jinja2, markupsafe, protobuf, protostruct, six):
    if not package.__file__.endswith("__init__.py"):
      manifest.append((package.__file__, package.__file__.split("/")[-1]))
      continue

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

  extensions_pb = "tangent/protostruct/descriptor_extensions_pb2.py"
  manifest.append((os.path.join(bindir, extensions_pb), extensions_pb))
  return manifest


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument(
      "-o", "--outfile",
      help="path to the output file to create")
  parser.add_argument(
      "-m", "--main",
      help="path to the output file to create")
  parser.add_argument(
      "-b", "--bin-dir", help="path to the binary directory")
  args = parser.parse_args()

  with zipfile.ZipFile(args.outfile, "w") as outfile:
    for srcpath, dstpath in get_manifest(args.bin_dir):
      outfile.write(srcpath, dstpath)
    outfile.writestr("google/__init__.py", "")
    outfile.writestr("tangent/__init__.py", "")
    outfile.write(args.main, "__main__.py")


if __name__ == "__main__":
  main()
