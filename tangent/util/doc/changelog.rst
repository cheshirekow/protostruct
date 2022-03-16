=========
Changelog
=========

-----------
v0.1 series
-----------

v0.1.0
======

dev0:
-----

* Initial version as a distinct package.
* Moved from /util to tangent/util

Closes: caa0169, 8f7dab5, 3d65ed6

-----------
v0.2 series
-----------

v0.2.2
======

* Add exception stream and assertion stream

v0.2.1
======

* Add string utils pop_prefix, pop_suffix, slice
* Expose split() to public API

v0.2.0
======

* add base64 util
* add refcounted library
* add array_stack util
* add binary tree of partial sums
* some refactor of the redblack tree to implement a node consistency model
  making it harder for library users to break their datastructures
