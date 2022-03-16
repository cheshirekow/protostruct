========
Features
========

.. default-role:: literal



--------------
Automatic Help
--------------

`argue` parsers can automatically pretty-print help text, such as this:

.. literalinclude:: bits/demo-usage.txt

This action is automatically added to the parser with the flags `-h/--help`
if the `add_help` metadata option is `true`.

-----------------------
Machine Parseable Help
-----------------------

The help information can be printed in JSON format instead of pretty text. The
JSON can then be processed to generate documentation pages (man pages or HTML).
Just export `ARGUE_HELP_FORMAT="json"` in the environment before using
`--help`. If the program uses subparsers, you may also wish to export
`ARGUE_HELP_RECURSE="1"` to include help contents for all subparsers
recursively.

The JSON help for the demo program is:

.. literalinclude:: bits/demo-usage.json

------------------------
Subcommands / Subparsers
------------------------

Argue supports arbitrary nesting of subcommands via
:code:`ArgumentParser::add_subparsers`. The API mirrors that of python's
`argparse`. See `examples/subparser_example.cc` for an example. The help text
for this example is:

.. literalinclude:: bits/subparser-example-usage.txt


-------------------------
Automatic Bash Completion
-------------------------

Any program which uses `argue` to parse it's command line arguments will
automatically supports bash autocompletion. The completion script can be found
at `bash_completion.d/argue-autocomplete`. Install this anywhere that bash
looks for completion scripts (e.g. `/usr/share/bash-completion` or
`~/.bash_completion.d` if the user is so configured). The script only needs to
be installed once, and completion will work for all `argue` enabled programs.

The completion script will detect if a program uses `argue` for it's argument
parsing and, if it does, it will call the program with some additional
environment variables signallying the :code:`ArgumentParser` to work in
completion mode instead of regular mode.

-------------------------------------
Unique Prefix Matching for Long Flags
-------------------------------------

If the program user provides a long flag (e.g. `--do-optional`) which does not
match any known long flags, but is a unique prefix of a known flag (e.g.
`--do-optional-thing`), then `argue` will match the known flag. The prefix must
be unique so `--do` will not match if `--do-optional-thing` and
`--do-other-thing` are both known.

