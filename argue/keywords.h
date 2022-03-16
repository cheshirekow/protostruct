#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdint>
#include <memory>

#include "argue/action.h"
#include "argue/exception.h"
#include "argue/parse.h"
#include "argue/util.h"

namespace argue {

// A unique integer used to label each of the keywords that the argue API
// accepts
enum TagNo {
  TAG_ACTION = 0,
  TAG_NARGS,
  TAG_CONST,
  TAG_DEFAULT,
  TAG_CHOICES,
  TAG_DEST,
  TAG_REQUIRED,
  TAG_HELP,
  TAG_METAVAR
};

// Associate a function argument with a compile-time tag
template <TagNo TAG, class T>
struct KeywordArgument {
  static constexpr TagNo keyword = TAG;
  T value;
};

// Keywords are no-storage objects which have an assignment operator
// that returns a tagged version of the assigned value.
template <TagNo TAG>
struct Keyword {
  template <class T>
  KeywordArgument<TAG, T> operator=(T value) const {
    return KeywordArgument<TAG, T>{value};
  }

  template <class T>
  KeywordArgument<TAG, std::vector<T>> operator=(
      const std::initializer_list<T>& value) const {
    return KeywordArgument<TAG, std::vector<T>>{value};
  }
};

// A keyword context is passed through the accumulation process
template <class T>
struct KeywordContext {
  std::shared_ptr<Action<T>> action;
};

// A struct with specializations for each keyword/tag
template <TagNo TAG>
struct AssignmentHelper {};

// Specialization for the "action" keyword, replaces the action with
// the assigned action.
template <>
struct AssignmentHelper<TAG_ACTION> {
  template <class T, class U>
  static void assign(KeywordContext<T>* ctx,
                     const std::shared_ptr<U>& argument);

  template <class T>
  static void assign(KeywordContext<T>* ctx, const char* named_action);
};

// Specialization for the "nargs" keyword, sets the nargs on the action
template <>
struct AssignmentHelper<TAG_NARGS> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, int value);

  template <class T>
  static void assign(KeywordContext<T>* ctx, const char* str);

  template <class T>
  static void assign(KeywordContext<T>* ctx, const char c);
};

// Specialization for the "const_" keyword, sets the const value on the
// action
template <>
struct AssignmentHelper<TAG_CONST> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, const T& value);
};

// Specialization for the "default_" keyword, sets the default value
// on the action
template <>
struct AssignmentHelper<TAG_DEFAULT> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, const T& value);

  template <class T>
  static void assign(KeywordContext<T>* ctx, const char* value);
};

// Specialization for the "choices" keyword. Sets the list of choices
// for the action
template <>
struct AssignmentHelper<TAG_CHOICES> {
  template <class T>
  static void assign(KeywordContext<T>* ctx,
                     const std::initializer_list<T>& choices);

  template <class T>
  static void assign(KeywordContext<T>* ctx, const std::vector<T>& choices);

  static void assign(KeywordContext<std::string>* ctx,
                     const std::vector<const char*>& choices);
};

// Specialization for the "dest" keyword. Sets the destination object
// where parsed values are stored.
template <>
struct AssignmentHelper<TAG_DEST> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, T* destination);

  template <class T, class Allocator>
  static void assign(KeywordContext<T>* ctx,
                     std::vector<T, Allocator>* destination);

  template <class T, class Allocator>
  static void assign(KeywordContext<T>* ctx,
                     std::list<T, Allocator>* destination);
};

// Specialization for the "required" keyword. Sets the required flag on
// the action object.
template <>
struct AssignmentHelper<TAG_REQUIRED> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, bool value);
};

// Specialization for the "help" keyword. Passes the string argument to the
// set_help method on the action.
template <>
struct AssignmentHelper<TAG_HELP> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, const std::string& value);
};

// Specialization for the "metavar" keyword. Passes the string argument to
// the set_metavar method on the actions.
template <>
struct AssignmentHelper<TAG_METAVAR> {
  template <class T>
  static void assign(KeywordContext<T>* ctx, const char* value);
};

// Handle a single keyword argument and perform the appropriate assignment on
// the action object.
template <TagNo TAG, class T, class U>
void HandleAssignment(KeywordContext<T>* action,
                      const KeywordArgument<TAG, U>& argument) {
  AssignmentHelper<TAG>::assign(action, argument.value);
}

// Base case of the reduction. With only one argument left to process, we
// need only process that single keyword.
template <class T, class Head>
void HandleSequence(KeywordContext<T>* ctx, const Head& head) {
  HandleAssignment(ctx, head);
}

// General case of the reduction. Given a sequence of more than one keyword
// arguments, perform the appropriate assignment of the first keyword argument,
// and then recurse on the remainder of the list.
template <class T, class Head, class... Tail>
void HandleSequence(KeywordContext<T>* ctx, const Head& head,
                    const Tail&... tail) {
  HandleAssignment(ctx, head);
  HandleSequence(ctx, tail...);
}

// Helper template used to infer the primitive type of the argument.
// The goal is to find the first keyword argument that implies the type of
// the command line argument. This is most likely a target destination
// but may also be a typed action object.
//
// In the general case (default template instanciation) provides a recursion,
// ignoring the first elemnt and recursing on the tail of the typelist.
template <class Head, class... Tail>
struct MakeHelper {
  typedef typename MakeHelper<Tail...>::ContextType ContextType;
  typedef typename MakeHelper<Tail...>::ElementType ElementType;

  static ContextType make_context() {
    return MakeHelper<Tail...>::make_context();
  }
};

// Specialization for an action keyword argument in the case that the action
// is a typed action object. In this case the action object is already
// instanciated with exactly the type we are looking for, so we use that
// type as the result.
template <class T, class... Args>
struct MakeHelper<KeywordArgument<TAG_ACTION, std::shared_ptr<Action<T>>>,
                  Args...> {
  typedef KeywordContext<T> ContextType;
  typedef T ElementType;

  static ContextType make_context() {
    return {std::make_shared<StoreValue<T>>()};
  }
};

// Specialization for an action keyword argument in the case that the action
// is a typed action object. In this case the action object is already
// instanciated with exactly the type we are looking for, so we use that
// type as the result.
template <class T>
struct MakeHelper<KeywordArgument<TAG_ACTION, std::shared_ptr<Action<T>>>> {
  typedef KeywordContext<T> ContextType;
  typedef T ElementType;

  static ContextType make_context() {
    return {std::make_shared<StoreValue<T>>()};
  }
};

// Specialization for a destination keyword argument. If we encounter such an
// argument it must be that the action was either defaulted or was a named
// action and so the type of the action has not been inferred yet. We
// consume the element type of the target destination as the primitive type
// of the action.
template <class T, class... Args>
struct MakeHelper<KeywordArgument<TAG_DEST, T*>, Args...> {
  typedef typename ElementType<T>::value ElementType;
  typedef KeywordContext<ElementType> ContextType;

  static ContextType make_context() {
    return {std::make_shared<StoreValue<ElementType>>()};
  }
};

// Specialization for a destination keyword argument. If we encounter such an
// argument it must be that the action was either defaulted or was a named
// action and so the type of the action has not been inferred yet. We
// consume the element type of the target destination as the primitive type
// of the action.
template <class T>
struct MakeHelper<KeywordArgument<TAG_DEST, T*>> {
  typedef typename ElementType<T>::value ElementType;
  typedef KeywordContext<ElementType> ContextType;

  static ContextType make_context() {
    return {std::make_shared<StoreValue<ElementType>>()};
  }
};

// Specializatinon for the case that we have exhausted all keyword arguments
// and still have not encountered a typed action object or a target
// destination. This means that the action must have no storage (e.g. is a void
// typed action).
template <TagNo TAG, class T>
struct MakeHelper<KeywordArgument<TAG, T>> {
  typedef void ElementType;
  typedef KeywordContext<void> ContextType;

  static ContextType make_context() {
    // TODO(josh): You can't instanciate StoreValue with a <void>. Need to
    // create a VoidAction dummy just to be a placeholder until the action
    // pointer is assigned.
    return {std::make_shared<StoreValue<void>>()};
  }
};

// Contains all of the keywords available to Parser::add_argument
namespace keywords {

constexpr Keyword<TAG_ACTION> action;
constexpr Keyword<TAG_NARGS> nargs;
constexpr Keyword<TAG_CONST> const_;
constexpr Keyword<TAG_DEFAULT> default_;
constexpr Keyword<TAG_CHOICES> choices;
constexpr Keyword<TAG_DEST> dest;
constexpr Keyword<TAG_REQUIRED> required;
constexpr Keyword<TAG_HELP> help;
constexpr Keyword<TAG_METAVAR> metavar;

}  // namespace keywords
}  // namespace argue
