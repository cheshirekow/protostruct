=====
Usage
=====

.. default-role:: code
.. highlight: c++

-------------------
Creating the parser
-------------------

To start with `argue`, create an `ArgumentParser` and use `add_argument` to
populate arguments. Then use `parse_args` to do the parsing.

.. literalinclude:: bits/simple_example.cc
  :language: C++

---------------------------
Specifying Argument Options
---------------------------

You can specify additional options to `add_argument` to configure how the
parser should handle that argument.

There are three different APIs for adding arguments. The first two are
experimental APIs designed to make your parser configuration code more
compact and readable. They are, however, a little "magic", so there is a
more intuitive fallback API.

Keyword API
===========

They keyword API is meant to mimic the keyword arguments of the `argparse`
python package. The `argue::keywords` namespace includes names of global
objects which act as the "keywords". Use it like this:

.. code-block:: cpp

  std::string foo;
  int bar;

  // Import into the active namespace the names of the keyword objects, e.g.
  // "action", "dest", "nargs', and "default_" used below.
  using namespace argue::keywords;

  // clang-format off
  parser.add_argument(
    "-f", "--foo", action="store", dest=&foo, help="Foo does foo things");
  parser.add_argument("-b", "--bar", dest=&bar, nargs="?", default_=1);
  // clang-format on

.. note::

   Because keyword arguments are not a common feature of C++ APIs, whatever
   beautifier you are using is likely to treat the keyword argument assigments
   as it would any other assignment. You may wish to locally disable your
   beautifier when using this API.

Kwargs Object API
=================

If your compiler supports non-trivial designated initializers (e.g. clang 5+,
or anything supporing c++20 designated initializers), then you can take
advantage of this API. Additional options are provided by passing a `KWargs<T>`
object after the destination argument. This object can be initialized
inline resulting in a similar appearance to the above:

.. code-block:: cpp

  parser.add_argument("--foo", &foo, {.help="Foo does foo things"});


Fallback API
============

The `add_argument` overloads all return an argument object. You can directly
assign the fields of this object to configure additional options.

.. code-block:: cpp

  auto arg = parser.add_argument("-f", "--foo", &foo);
  arg.help = "Foo does foo things";

Additional Notes
================

You don't have to specify the destination inline as an argument after the name
or flags. You could specify it as a keyword argument or as an assignment with
the fallback API, however then the type of the argument cannot be inferred
during the call to `add_argument` and it must be provided explicitly. For
example:

.. code-block:: cpp

   auto arg = parser.add_argument<int>("-f", "--foo");
   arg.dest = &foo;

This is true for the KWargs object API and the fallback API, but is not
required by the keywords API.

--------------------------
Supported Argument Options
--------------------------

.. note::

  Much of the the text in this section is borrowed from
  https://docs.python.org/3/library/argparse.html


nargs
=====

`.action=` may be either a `shared_ptr` to an `Action<T>` object, or it may
be one of the following strings.

* `"store"` -  This just stores the argument’s value. This is the default
  action.
* `"store_const"` - This stores the value specified by the const keyword
  argument. The 'store_const' action is most commonly used with optional
  arguments that specify some sort of flag. For example:
* `"store_true"` and `"store_false"` - These are special cases of
  `"store_const"` used for storing the values True and False respectively. In
  addition, they create default values of False and True respectively.
* `"help"` - This prints a complete help message for all the options in the
  current parser and then exits. By default a help action is automatically
  added to the parser. See ArgumentParser for details of how the output is
  created.
* `"version"` - This expects a `.version=` keyword argument in the
  `add_argument()` call, and prints version information and exits when invoked

nargs
=====

ArgumentParser objects usually associate a single command-line argument with a
single action to be taken. The nargs keyword argument associates a different
number of command-line arguments with a single action. The supported values
are:

* `N` (an integer). `N` arguments from the command line will be gathered
  together into a list. Note that nargs=1 produces a list of one item. This is
  different from the default, in which the item is produced by itself.
* `"?"`. One argument will be consumed from the command line if possible, and
  produced as a single item. If no command-line argument is present, the value
  from default will be produced. Note that for optional arguments, there is an
  additional case - the option string is present but not followed by a
  command-line argument. In this case the value from const will be produced.
* `"*"`. All command-line arguments present are gathered into a list. Note that
  it generally doesn’t make much sense to have more than one positional
  argument with nargs='*', but multiple optional arguments with nargs='*' is
  possible.
* `"+"`. Just like `"*"`, all command-line args present are gathered into a
  list. Additionally, an error message will be generated if there wasn’t at
  least one command-line argument present. For example:
* `argue::REMAINDER`. All the remaining command-line arguments are gathered
  into a list. This is commonly useful for command line utilities that dispatch
  to other command line utilities.

If the nargs keyword argument is not provided, the number of arguments consumed
is determined by the action. Generally this means a single command-line
argument will be consumed and a single item (not a list) will be produced.

Note that for `nargs` that imply a list of arguments, the destination object
must be of a supported container type (e.g. `std::list` or `std::vector`).

const
=====

The const argument of `add_argument()` is used to hold constant values that are
not read from the command line but are required for the various ArgumentParser
actions. The two most common uses of it are:

* When `add_argument()` is called with `.action="store_const"` or
  `.action="append_const"`. These actions add the const value to one of the
  attributes of the object returned by parse_args(). See the action description
  for examples.
* When `add_argument()` is called with option strings (like -f or --foo) and
  `nargs="?"`. This creates an optional argument that can be followed by zero
  or one command-line arguments. When parsing the command line, if the option
  string is encountered with no command-line argument following it, the value
  of const will be assumed instead. See the nargs description for examples.

With the `"store_const"` and `"append_const"` actions, the const keyword
argument must be given. For other actions, it defaults to None.

default\_
=========

All optional arguments and some positional arguments may be omitted at the
command line. The default keyword argument of `add_argument()`, whose value
defaults to None, specifies what value should be used if the command-line
argument is not present. For optional arguments, the default value is used when
the option string was not present at the command line.

Note that in C++ `default` is a reserved word so this keyword ends with an
underscore ('_').

choices
=======

Some command-line arguments should be selected from a restricted set of values.
These can be handled by passing a container object as the choices keyword
argument to `add_argument()`. When the command line is parsed, argument values
will be checked, and an error message will be displayed if the argument was not
one of the acceptable values

required
========

In general, `argue` assumes that flags like `-f` and `--bar` indicate optional
arguments, which can always be omitted at the command line. To make an option
required, `true` can be specified for the `required=` keyword argument to
`add_argument()`.

help
====

The help value is a string containing a brief description of the argument. When
a user requests help (usually by using `-h` or `--help` at the command line),
these help descriptions will be displayed with each argument.

metavar
=======

When `ArgumentParser` generates help messages, it needs some way to refer to
each expected argument. By default, for arguments which have a flag, the flag
name is used. Positional arguments have no default. In either case, a name
can be specified with `metavar`.

dest
====

Most `ArgumentParser` actions store some value to some variable. The address
of the variable to store values can be specifie dwith this keyword argument.

-------------
Demonstration
-------------

There are a couple of examples in the :literal:`examples/` directory of the
source package. For example, here is a replica of the demo application from the
python `argparse` documentation, written in C++ using `argue`:

.. literalinclude:: bits/demo.cc
  :language: C++

When executed with ``-h`` the output is:

.. literalinclude:: bits/demo-usage.txt

-------------------------------
Note on designated-initializers
-------------------------------

Designated initializers are a `C99` feature
(as well as an upcoming `C++20` feature) that `clang` interprets
correctly (as an extension) when compiling `C++`, but is not in fact a
language feature. The `GNU` toolchain does not implement this feature.
Therefore, while the following is valid when compiling with `clang`::

    parser.add_argument("integer", &int_args, {
      .nargs_ = "+",
      .help_ = "an integer for the accumulator",
      .metavar_ = "N"
    });

We are not allowed to skip any fields in GCC, meaning that if we wish to
use designated initializers in GCC, we must use the following::

    parser.add_argument("integer", &int_args, {
      .action_ = "store",
      .nargs_ = "+",
      .const_ = argue::kNone,
      .default_ = argue::kNone,
      .choices_ = {1, 2, 3, 4},
      .required_ = false,
      .help_ = "an integer for the accumulator",
      .metavar_ = "N",
    });

Alternatively we could use the more brittle universal initializer syntax
with no designators::

    parser.add_argument("integer", &int_args, {
      /*.action_ =*/ "store",
      /*.nargs_ =*/ "+",
      /*.const_ =*/ argue::kNone,
      /*.default_ =*/ argue::kNone,
      /*.choices_ =*/ {1, 2, 3, 4},
      /*.required_ =*/ false,
      /*.help_ =*/ "an integer for the accumulator",
      /*.metavar_ =*/ "N",
    });

But this can get pretty tedious. Therefore, unless you're limited to a
compiler supporting designated initializers in `C++` you may wish to stick to
the alternative assignment APIs.

