"""
A test which passes if two files are the same, and fails if they differ

Compare two files and exit with nonzero retcode if they differ. Print the
difference to stderr.
"""

import argparse
import difflib
import shutil
import sys

argparser = argparse.ArgumentParser(description=__doc__)
argparser.add_argument(
    "--truth-file",
    help="The file containing the true contents")
argparser.add_argument(
  "--query-file",
  help="The file containing content which should match truth_file")
argparser.add_argument(
  "--fix", action="store_true",
  help="If the teset would otherwise fail, then copy truth_file to query_file"
       " and exit with success")
args = argparser.parse_args()


with open(args.truth_file, "r") as infile:
  truth_lines = infile.read().split("\n")

query_lines = []
try:
  with open(args.query_file, "r") as infile:
    query_lines = infile.read().split("\n")
except FileNotFoundError:
  pass

difflines = list(
  difflib.unified_diff(
    query_lines, truth_lines, fromfile=args.query_file, tofile=args.truth_file,
    n=3, lineterm='\n')
)

if not difflines:
  sys.exit(0)

if not args.fix:
  for line in difflines:
    sys.stderr.write(line)
    sys.stderr.write("\n")
  sys.exit(1)

shutil.copy(args.truth_file, args.query_file)
sys.exit(0)
