===========
Protostruct
===========

.. default-role:: literal

Protostruct is a code generator to generate serialization bindings for native
C structs. Currently it can generate serializations for
google's `protocol buffer`__ (protobuf) as well as `cereal`__ bindings
(yielding support for JSON, XML, etc).

Use `protostruct` to add modern extensible serialization and over-the-wire
messaging to your existing native "pure" C APIs.

Protostruct works in two steps `compile` and `gen`.

.. __: https://developers.google.com/protocol-buffers
.. __: http://uscilab.github.io/cereal/

-------------------------------
compile (`FileDescriptorProto`)
-------------------------------

`protostruct` can process a C header file (`foo.h`) and generate a binary
description of corresponding google protocol buffers for each enum or struct
defined in that header file. This binary description is in the form of a
serialized `FileDescriptorProto` (with some extensions) and is largely an
intermediate format.

If a corresponding .proto file already exists, protostruct can parse it and
use it to provide hints for certain decisions in the conversion process. In
this way, `protostruct` can be used to keep a `.h` and a `.proto` in sync.
In particular, the following will be processed from the existing `.proto`:

1. includes and `package` declaration are preserved
2. zigzag or fixed-sized fields or numeric types which are larger than
   the corresponding C field are preserved (the default output is is `varint`
   of the same size)
3. any field whose type has changed is given a new field number, and the old
   number is added to the `reserved` list for that message
4. any field which has been removed has it's number added to the `reserved`
   list
5. if a primitive repeated field is annotated as "packed" then that option is
   preserved.

--------
generate
--------

From the intermediate description, `protostruct` can generate four types of
code.

foo.proto
=========

This is a `protobuf` message description corresponding to the the structs and
enumerations defined in the `.h` file.

foo.pbc2.[h|cc]
===============

For each enumeration or message in the original `.h` file, these files will
include a conversion function that will deep-copy a message to/from a C++
message type generated by the output of `protoc` on the above generated
`.proto` file. Use this for dumb synchronization between C and C++
representations of the message data.

foo.pbwire.[h|c]
================

These files include high-efficency serialization and deserialization of the
message directly between the C structures and the protobuf wire format. These
conversions depend on the lightweight wire-format library `libpbwire`
(included in this project).

foo.cereal.h
============

This file contains `cereal` bindings (`[load|save]_minimal(...)` for enums,
and `serialize(...)` for structures).

--------
Examples
--------

See the examples in `tangent/protostruct/test`.

* `test_messages.h` demonstrates some plain `C` enumerations and structures
  which we want to serialize.
* `test_in.proto` represents an "out of date" protobuf description of those
  enums and structs.
* `test_messages.proto` shows the up-to-date synchronized protobuf description
  as the result of running `protostruct` on `test_messages.h` and
  `test_in.proto`
* `test_messages.pb2c.[h|cc]` demonstrates the generated conversion routines
  between the original C structures and the C++ message types generated by
  `protoc` from the `.proto`.
* `test_messages.pbwire.[h|c]` demonstrates the generated
  serialization/deserialization functions which work directly between the
  C structures and the protobuf wire format.
* `test_messages.cereal.h` demonstrates the generated cereal bindings.


------------
Installation
------------

You can install from the `tangent ppa`__::

  ~# add-apt-repository ppa:josh-bialkowski/tangent
  ~# apt install protostruct

.. __: https://launchpad.net/~josh-bialkowski/+archive/ubuntu/tangent
