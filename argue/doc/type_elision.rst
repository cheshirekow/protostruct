============
Type Elision
============

Using templates in argue has gotten rather combersome and I feel like it's
cluttering up the API. At the end of the day the type doesn't really matter
for most of what we're doing, so can we design the system in such a way that
types are elided and somehow buried underneith a cleaner typeless interface?

Furthermore, can we get rid of the need to provide the target pointer in the
positional argument list, and rely on the kwargs (first kwarg?). Some flags
are pure action and have no storage.

Furthermore, the original data is always a string, so all the type information
that we have in the options structure seems un-necessary. The current kwargs
object is::

    template <typename T = NoneType>
    struct CommonOptions {
      typedef typename get_choice_type<T>::type U;

      ActionHolder<T> action_;
      Nargs nargs_;
      Optional<T> const_;
      Optional<T> default_;
      std::list<U> choices_;
      bool required_;
      std::string help_;
      std::string metavar_;
      T* dest_;
    };

``Actions`` are typed (and therefore ``ActionHolders`` because they store an
options object and generally address their destination directly. If we wrap the
destination in a ``Parser`` object which has a ``ConsumeArg`` method then we
can get rid of the typed ``T*`` `dest_` pointer. This will also be a bit more
flexible in that alternate "destinations" can be provided.

The ``Store`` action just parses the string into the destination so wrapping
the desination with a generic ``Parser`` will be enough to cover this action,
however dealing with "default" and "choices" can be complicated. In general,
however, all of "dest", "const", "default", and elements of "choices" *could*
ideally be the same type.

What if, instead of actually storing the data in the fields, the fields are
objects whose assignment operator is overloaded. See ``idea_test.cc`` for an
example. This seems to work great for ``nargs`` (integer) and presumably
will work just fine for strings (help, metavar), and bool (required). But
what about const, default, and choices?

One idea is to store these values in a variant type, which includes some kind
of runtime type id. The action object can then lookup the typeid and as long as
it recognizes it, it can correctly cast the stored pointer and do what it needs
to do with it.

For ``const`` and ``default`` though, the type of the stored object should
(probably?) be the same as the type of the destination object... or at least
it should be easily convertable to it. For these, instead of passing them into
the action, can we pass them into the destination holder? Then the action would
call something like this::

    destination->AssignDefault();
    destination->ParseArg(arg);

for a ``StoreAction`` or something like this::

    destination->AssignDefault();
    destination->AssignConst();

for a ``StoreConstAction``.

Another idea is, instead of storing values for ``const`` and ``default``,
instead store assignment lambdas which take a pointer to the destination. This
would extend customizability by allowing arbitrary lambas that can reuse the
``StoreConst`` framework to generate complex actions (think JSON args).

Expanding on the idea of utilizing type erasure:
``dest`` is an shared pointer of an untyped base class to an object which
is templated. ``const``, and ``default`` store a type-erased holder of
some data. The typeid is stored in the object and when either a const value
or a default value is applied to the target variable, the typeid is compared
with the target at runtime. The runtime looks up an appropriate parser using
the typeid pair. If no runtime converter is found and assignment operator is
callable, then it will be called.

Both ``dest`` and ``const`` may store either a scalar value or a lambda.

Alternative idea is to just store ``dest`` and ``const`` as strings... which
is much simpler to implement. It sacrifices some small amount of type-safety
but since we would erase types anyway all of that would be encountered during
runtime. We could provide an overload for string types and then havea template
which accepts anything but uses libfmt (and therefore, ostream operator) to
actually format to a string. This doesn't allow for default/const of something
like ``nullptr_t`` though. But then again, the only destination that would
make sense for is a pointer, but we can't fill a pointer with a string for
the default actions anyway.

Choices could be similar, accepting any standard container or c style array,
and then iterating over it and storing a list of strings. Alternatively a
function taking a string.

If the kwargs object were typed then we would change it to::

    template <typename T>
    struct TypedKwargs {
      std::function<void(T*)> const_;
      std::function<void(T*)> default_;
      std::function<bool (const char*)> choices;
      T* dest;
    };

The only thing this extends beyond what we already have is the ability to
abue ``store_const`` to execute a complicated initializtaion function, which
should probably just have it's own action in that case. Yeah, in fact, writing
an action which does all of this is super easy, really we just want to support
the argparse convenience stuff. In which case, storing functions is overkill,
and I think type-erasure is a totally fine price to pay.

Yet another idea along the lines of type erasure is to use a variant type.
We could cover all the built in types, plus pointers (maybe smart pointers).

Oh but this is C++... so now I'm back to the argument that "we (generally)
have to store the argument somewhere and we can't createa dynamic map (well,
we could but that would largely defeat the purpose). Therefore we actually
know the type of the destination because it's a manditory parameter (unless
the action is terminal, like version or help).

Therefore we can safely type the kwargs object.

Ok, new thought... let's say we're using some kind of variant class with
type erasure to store default, const, and maybe choices. Now, if I'm a user
and I'm writing a custom action, then I'm also supplying the input to that
action. If I choose to reuse this variant infrastructure then I already
know what type to expect... so I shouldn't be surprised by any inputs and
it's reasonable to just failout at runtime if I get an unexpected type.

Otherwise, for the builtin actions, we can just provide default conversion
from all the primitive types back and forth to each other. We *might* want
to store the conversion functions in a runtime registry in the parser object
so that that the user can extend it... though I really can't think of a case
where that makes a ton of sense. Maybe just for something like a ```Rational``
or a ``Pair`` type, which is a very low-level (almost primitive) type and maybe
they want to be able to convert native scalars to/from.
