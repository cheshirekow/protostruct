with open("./conf_common.py") as infile:
  exec(infile.read())  # pylint: disable=W0122

project = "protostruct"
docname = project + u'doc'
title = project + ' Documentation'
version = "@PROTOSTRUCT_VERSION@"

assert version is not None
release = version
