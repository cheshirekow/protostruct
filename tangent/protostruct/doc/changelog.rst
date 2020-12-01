=========
Changelog
=========

-----------
v0.1 series
-----------

v0.1.0
======

Initial release

* Uses libclang8 to walk the translation unit for a header file
* Uses libprotoc to parse existing .proto into structured data
* Emitters in python using jinja2 for

  * updated .proto
  * conversion between C structs and protobuf C++ messages
  * direct serialization from C struct to protobuf wire format

* Includes debian package build
* Includes sparse export configuration

v0.1.1
======

* Add cereal-binding emitters
* pass a pointer to the parse/emit context (instead of a copy) for pbwire APIs
* cleanup logging/messages
* Store the C typename in the field descriptor
* Split the two passes for emit into two separate functions
* Prevent double-visit of decl cursors


