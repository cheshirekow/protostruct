=================================
Protostruct Detailed Design Notes
=================================

.. default-role:: code

.. contents:: Table of Contents
  :depth: 2
  :class: toc

------------------
Motivating Usecase
------------------

The design of protostruct is pretty general, so there are a couple of
ways one may choose to use it. In order to keep this discussion concrete,
let us consider the following usecase:

.. admonition:: Motivating Usecase
   :name: motivating-usecase
   :class: usecase

   1. Our project has an existing set of C `structs` and `enums` which we wish to
      use as *messages* in some messaging system.

   2. These C `structs` are plain data with no pointer fields. All repeated
      fields are implemented by a fixed length c array

   3. Dynamically sized repeated fields are implemented by a fixed length
      c array, where the static length is interpreted as the capacity of that
      field, and a separate integer field holds the occupied length of that
      field.

In order to promote these structures into proper *messages* protostruct provides
three functions:

1. Compile the C `structs` and `enums` into a serialized protocol buffer
   descriptor, making judicious choices for wire formats and using some
   protocol buffer extensions to annotate the descriptors with hints regarding
   the C representation of the data.
2. Reverse a textual `.proto` file from the serialized descriptor
3. Generate plain C language bindings from the serialized descriptors which are,
   more-or-less identical to the original `structs` and `enums`.

In the remaining sections, we will discuss different features of protostruct,
how those features support this usecase, and design details as they relate to
those features.

-----------
Compilation
-----------

Input / Output
==============

Conceptually, the input of the protostruct compilation process is a
*translation unit* and the output is a serialized `FileDescriptorSet`
(see `descriptor.proto`__). Protostruct's basic assumption is the following:

.. admonition:: Assumption

  Some `enums` and `structs` within the input translation unit are in fact
  C language bindings of protocol buffer messages.

The purpose of the compilation is to reconstruct the *generic* protocol buffer
representation from the C language bindings. This generic representation can
then be used to generate bindings for other languages, or to extend the
existing C-language bindings with additional functionality, such as
serialization routines.

.. __: https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/descriptor.proto

Synchronizing protobuf definitions
==================================

Protostruct may optionally take as input a serialized `FileDesciptorSet` to
synchronize. Semantically, `enums` and `structs` found in the input translation
unit which match the filter set are treated as a more up-to-date representation
of messages than those that are found in the input `FileDescriptorSet`.
Anywhere that the C representation may be valid bindings for multiple protobuf
definitions, an existing representation in the input `FileDescriptorSet`
is chosen over the default. As a specific example, consider a case where
protostruct parses a struct declaration like:

.. code:: c++

  struct FooMessage {
    int32_t field_a;
    int32_t field_b;
    int32_t field_c;
  };

with an input FileDescriptorSet containing somewhere the serialized `Descriptor`
matching:

.. code:: proto

  message FooMessage {
    reserved 1-10;

    sint32 field_a = 11;
    fixed32 field_b = 12;
  };

In this case, protostruct understands that the `FooMessage` struct is an
updated version of the existing `FooMessage` with the addition of `field_c`.
It will record in the output all three fields. For the two fields which were
present in the synchronization input, the types will be preserved (`sint32`
and `fixed32`) because both wire formats are compatible with the C type
`int32_t`. The third field will be recorded with a proto type of `int32` which
is the default mapped type for a c type of `int32_t`. Additionally, protostruct
will assign a field id of `13` to the newly added field.


Processing the C language
=========================

Protostruct uses `libclang` to compile the translation unit and it's
`visitor API`__ to inspect the Abstract Syntax Tree (AST). The top level
`FileVisitor` just recurses on all AST nodes until it finds one of the
following cursors of interst, at which point it dispatches a more specific
visitor (of the type indicated) or makes a record of the entry.

.. table::

  +-------------------+------------------+--------------------------------------+
  | Cursor of Interst | Specific Visitor | Semantics                            |
  +===================+==================+======================================+
  | InclusionDirective| N/A              | Record the included header file so   |
  |                   |                  | that it may be converted to a .proto |
  |                   |                  | inclusion if it contains messages    |
  +-------------------+------------------+--------------------------------------+
  | EnumDecl          | EnumVisitor      | Transcribe the `enum` definition     |
  |                   |                  | to a proto `EnumDescriptor`          |
  +-------------------+------------------+--------------------------------------+
  | StructDecl        | MessageVisitor   | Transcribe the `struct` definition   |
  |                   |                  | to a proto `Descriptor`              |
  +-------------------+------------------+--------------------------------------+
  | MacroDefinition   | N/A              | Record macros which expand to        |
  |                   |                  | integer constants which are used as  |
  |                   |                  | the capacity of a repeated field     |
  +-------------------+------------------+--------------------------------------+

The `EnumVisitor` iterates over the enumerator constant declarations
(`EnumConstantDecl`) and populates an `EnumValueDescriptorProto`
(`definition`__) for each one.

The `MessageVisitor` iterates over fields of the struct declaration
(`FieldDecl`) and populates a `FieldDescriptorProto` (`definition`__) for
each one.

.. __: https://clang.llvm.org/doxygen/group__CINDEX__CURSOR__TRAVERSAL.html
.. __: https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/descriptor.proto#L276
.. __: https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/descriptor.proto#L138

.. _synchronization-rules:

Synchronization Rules
=====================

The synchronization is similar, in general, between `enums` and `messages`.
In either case, protostruct will inspect the name of the header file in which
the definition resides. It will then construct the correlated `.proto` filename
by replacing the `.h` extension. Then it will check to see if the input
`FileDescriptorSet` contains a `FileDescriptor` with this `.proto` filename.
If it does, then this `FileDescriptor` is held as the "current" `FileDescriptor`
as protostruct visits declarations within this file. If it does no exist, a
new empty `FileDescriptor` is constructed and made current.

For each `enum` or `struct` visited, protostruct will check to see if the
current `FileDescriptor` already contains an `EnumDescriptorProto` or
`DescriptorProto` with the same name. If it does, then this descriptor is made
current for the nested visitation, otherwise a new descriptor is constructed.

As the contained items (enumerators for `enum` or fields for `struct`) are
visited in order if there already exists a descriptor matching the visited
item (by name) and that descriptor is compatible with the current C definition,
then the existing descriptor is retained. If it is not compatible, the existing
descriptor number is retired, and a new number is allocated from the available
pool.


.. _matching-field-types:

Matching field types
====================

The mapping of protocol buffers wire format to C primitive types is not
one-to-one. For example, protocol buffers defines four different ways of
representing a signed 32-bit integer (while C has just `int32_t`). Likewise,
protocol buffers has no representation for integers less than 32-bits wide,
(while C has, e.g. `int8_t` and `int16_t`). This means that we are left with
some choices when generating C bindings or when reversing the descriptors from
existing C structures.

Protostruct maintains (in `get_compatible()`) a set of compatible protocol
buffer types (`google::protobuf::FieldDescriptorProto_Type`) for each
numeric primitive (`CXType`) in C. When visiting a field declaration,
protostruct will look up the compatible proto types. If a matching
`FieldDescriptor` already exists for the visited field, and it's current
type is compatible with the C type, then the existing `FieldDescriptor` is
left in place. If it is not compatible, then the existin field number is
retired, adding it to the reserved set, and a new field number is assigned to
the `FieldDescriptor`, and it is given the default compatible proto wire
type.


Matching repeated fields to their length
========================================

When processing a repeated field (`CXType.kind == CXType_ConstantArray`)
protostruct will do a couple special things. First, it will inspect the size of
the array and record this as the `FieldDescriptorProto.FieldOptions.capacity`
extension (see `set_field_type()`). Second, protostruct will walk the AST
subtree for this cursor and it finds an integer literal cursor which matches the
name of a macro defined in the translation unit, it will record this name as the
`FielDescriptorProto.FieldOptions.capname` extension (see `FieldVisitor`).

When a `struct` visitation is complete, protostruct will attempt to correlate
length fields with repeated fields. A length field is an integer typed field
whose purpose is to store the filled size of a "dynamic" repeated field.
`protostruct` will match any integer typed field named `FooCount` or `Foo_size`
if there is also a repeated field named `Foo` in the same struct.

Storing Comments
================

A `FileDescriptorProto` may contain information about the source file that it
parsed. If it does, this information is stored in `SourceCodeInfo`
(see `definition`__). This is where protoc stores comments that it processes
for enums, enumerators, messages, fields, etc. Note that this field lives at
the root of the `FileDescriptorProto` meaning that comments are not stored
along with their corresponding proto, but in this separate field at the top
of the message structure. The mapping between source info and the item that
it corresponds to is a little opaque, and is based on a "message path". This
path is a list of integers and corresponds to the field numbers that would
encountered when walking the `FileDescriptorProto` message. For Example,
"path" to the first enumerator within the second enum is `{5, 1, 2, 0}` where
the semantics are:

* 5: is the field id of `enum_type`
* 1: the second entry within the `enum_type` repeated field
* 2: is the field id of `value`
* 0: the first entry within the `value` repeated field

Protostruct will retrieve comments for each item that it processes using
`libclang` `clang_Cursor_getRawCommentText()`, which seems to do a very good
job of associating the most relevent comments. It will attempt to
strip comment prefixes (e.g. `//!<`) from comment lines. Then it will store the
comment text as leading comment within `SourceCodeInfo`.

.. __: https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/descriptor.proto#L766


---------------------
Descriptor Extensions
---------------------

Protostruct defines a number of descriptor extensions in order to assist the
code generator in creating the native C-bindings. If `protostrut compile`
is used to generate the .proto definition, then these extensions are
populated during inspection of the AST in order to record the various choices
that are observed.

File Options
============

Capacity Macros
---------------

In many cases it is convenient to use a named identifier when refering to the
capacity or size of a repeated field. This especially occurs in the case that
multiple repeated fields are correlated, such that item "i" in one field
corresponds to item "i" in the other field. In the C bindings, protostruct
supports the use of preprocessor macros for defining these constants. The
macro definitions are stored in `capacity_macros` `FileOption`.
`protostruct compile` will populate these as it processes the translation unit,
or they may be manually entered in the .proto file. `protostruct gen` will
faithfully transcribe them to the top of the generated `.h` file which
corresponds to a given .proto

Field Options
=============

Field Type
----------

See the discussion in matching-field-types_ regarding the mapping of C to
proto field types. The choice of which C primitive type to use, if not the
default for a given protobuf wire format primitive, is recorded as a string in
the `fieldtype` extension of the `FieldOptions` proto.

Length Field
------------

The pattern employed by the C language bindings for repeated fields is
that such a field is represented in C by a fixed-sized C array. The fixed size
represent's it's capacity, and it's occupied size, or length, is stored in a
separate integer field. The `lengthfield` extensions of the `FieldOptions`
proto is used to store the name of the integer field which holds the
occupied length.

Capacity
--------

The `capacity` extension of the `FieldOptions` proto stores the fixed size
of the c-array for any repeated field (see the section above). This extension
is an integer type, and is used when the capacity is intended to be an integer
literal.

Capacity Name
-------------

The `capname` extension of the `FieldOptions` proto is similar to `capacity`,
but it's type is a string (instead of an integer). This extension is used when
the capacity of a repeated field is determined by a C constant (e.g. `#define`
macro or `enum`).

----------------
Code Generations
----------------

Protostruct uses python and `jinja2` (see e.g. `here`__) to generate code.
Note that protoc is extensible via it's `plugin API`__ and recommends that
plugins adhere to the API to yield a consistent user interface. Protostruct
does not currently due this, but the adaptation is not particularly significant.
For now, protostruct is focused on just generating the code.

.. __: https://jinja.palletsprojects.com/en/3.0.x/
.. __: https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.compiler.plugin.pb

The architecture is relatively straightforward. `protostruct generate` uses the
protoc generated bindings for the descriptor protos. It takes as input a
serialized `FileDescriptorSet`, which it will parse into a native python
object. It then instantiates the appropriate jinja template for the code that
it is generating, and then renders the template once for each file represtend
in the `FileDescriptorSet`. For each rendering, the `FileDescriptorProto` is
passed to the jinjra environemnt. The jinja template will iterate over options,
enums, and messages defined in the `FileDescriptorProto` and generate code
for each one.

In addition to the `FileDescriptor`, the generator also provides in the jinja
environment a context object which implements a number of utility functions.
We will review the codegen utilities a bit later.

C-Structures
============

The `XXX-recon.h.jinja2` template *reconstructs* the C bindings from the
`FileDescriptorProto`. The template is pretty straightforward (only 50 lines
long) as most of the work is just transcription from the proto representation.

Direct proto-wire serialization
===============================

The `XXX.pbwire.h.jinja2` and `XXX.pbwire.c.jinja2` templates generate
serialization code which serialize C structures directly to protocol buffer
wire format. These bindings generate two functions for each message or enum
`Foo` (for enum types the value parameter is by-value, not a pointer):

1. `pbemit_Foo(pbwire_EmitContext* ctx, Foo* value);`
2. `pbparse_Foo(pbwire_ParseContext* ctx, Foo* value);`

`pbewire_EmitContext` and `pbwire_ParseContext` are very simple structures
which maintain begin/current/end pointers to the (read/write) buffer for
serialized data, error buffer, and some accounting data. Emission is done in
two passes. The code for each pass is pretty much the same, but in the first
pass is used to compute the length of the output messages while the second
pass actually writes the data. This is only necessary if length-delimited fields
record the delimited length as varint, and emission could presumably done in
a single pass if a fixedint is used (with potentially worse cache
coherence).

The generated code for each emission pass and for parsing is pretty
straightforward. Enums are emitted on the wire as `int32`. When they are parsed,
they are parsed as `int32` and then a `switch` statement resolve the enumeration
that the value corresponds to. Structs are emitted field-by-field, calling the
associated emission function for the field type. Structs are parsed via
`pbwire_parse_message` which handles the wire protocol of field-id/value pairs.
It takes a `_fielditem()` callback function which is dispatched for each field
which is encountered. The callback is generated code and is generally just a
`switch` statement which cases each field id to parses the data into struct
member corresponding to that field.

Cereal bindings for JSON, XML
=============================

The `XXX.cereal.h.jinja` can generate serialization templates for the `cereal`
library (which, in turn, comes with backends for JSON and XML). The template
for these bindings is very uninteresting because most of the type-magic is
implemented in cereal itself. The generated
`serialize(Archive& archive, Foo& obj)` template basically just calls
`archive()` on each field.