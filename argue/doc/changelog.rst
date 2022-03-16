=========
Changelog
=========

-----------
v0.1 series
-----------

v0.1.4
======

dev0:
-----

* Remove dependency on libtangent-json
* Implement "append" action

v0.1.3
======

dev0:
-----

* On error, usage string is appended to exception message, meaning that
  subparser usage is printed rather than superparser when subparser is
  missing a required argument.
* Fix buffer underflow in subparser action when invalid subcommand is used
* Argue --help can now dump JSON instead of text format.
* Argue programs now inherently suports bash autocompletion
* Registering the same action twice will now throw an exception
* AddArgument returns the kwarg object, meaning that fields can be set after
  the call instead of during (for compilers like GCC which don't support
  designated initializers.

Closes: 405abc1, 4f5e576, db38521, e95f5f5

dev1:
-----

* Split argue.h/argue.cc into separate files based on concepts
* Remove argue utilities duplicated in util/
* Switch to lower_snake_case method style
* Metadata version number is a string
* Get rid of ARGUE_EMPTY macros

Closes: 0310925, 8d56785

dev2:
-----

* Implement keywords API
* Unify StoreScalar and StoreList into StoreValue
* Assigning to nargs will not replace the action
* If the program user provides a long flag which is not an exact match but is
  a unique prefix of a known flag, then match that flag.
* Pass column size into get_help() and get_prolog() so that actions can
  format their own help text.

Closes: 41e5630, 504d241, 73c5d7a, aead983, d5c7d88

dev3:
-----

* Implement debian package build

Closes: 51f1ef7

dev5:
-----

* Add support for using const char* as a `default=` for a std::string
  modeled variable.

v0.1.2
======

* Add support for ``--version`` and ``--help`` without the corresponding short
  flag (i.e. no ``-v`` or ``-h``)
* Add macro shims to work with gcc which doesn't support designated
  initializers
* Add support for ``nargs=REMAINDER``
* Added ``argue``/``glog`` integration via a function to add command line
  flags for all the ``glog`` ``gflag`` globals
* Did some build system cleanup
* Removed individual exception classes, unifying them into a single one
* Replace hacky assertion stream with ``fmt::format()`` usages.
* Replace KWargs class with optional containers with KWargs field objects
  that pass-through to setters instead.
* Don't latch help text at help tree construction time, instead query help
  out of the action objects at runtime. This is so that subparsers know what
  children they have and can generate choices text.

v0.1.1
======

* Implemented subparser support

v0.1.0
======

Initial release! This is mostly a proof of concept initial implementation. It
lacks several features I would consider required but works pretty well to start
using it.

Features:

* argparse-like syntax
* type-safe implementations for ``store``, ``store_const``, ``help``, and
  ``version`` actions
* support for scalar ``(nargs=<default>, nargs='?')`` or
  container (nargs=<n>, nargs='+', nargs='*') assignment
* provides different exception types and error messages for exceptional
  conditions so they can be distinguised between input errors (user errors),
  library usage errors (your bugs), or library errors (my bugs).
* support for custom actions
* output formatting for usage, version, and full help text
