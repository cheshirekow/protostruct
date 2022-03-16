import os

# Source the common stuff
with open(os.path.join("./conf_common.py")) as infile:
  exec(infile.read())  # pylint: disable=W0122

project = "argue"
docname = project + u'doc'
title = project + ' Documentation'
version = "@ARGUE_VERSION@"
