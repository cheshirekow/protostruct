#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "argue/action.h"

// An command line argument parsing library
namespace argue {

// =============================================================================
//                              KWargs
// =============================================================================

// Provide keyword-argument assignment syntax to the various setters of actions
/* TODO(josh): Make field classes non-constructable outside the KWargs class.
 * NOTE(josh): we need to delete assignment operators (here an below) so
 * that the compiler doesn't automatically match the copy assignment
 * operator for argument types that are construction-convertable to this
 * type */
template <typename T>
class KWargs {
 public:
  class ActionField : public std::shared_ptr<Action<T>> {
   public:
    ActionField();
    ActionField(
        const std::shared_ptr<Action<T>>& action);  // NOLINT(runtime/explicit)
    ActionField(const char* named_action);          // NOLINT(runtime/explicit)
    ActionField(const std::string& named_action);   // NOLINT(runtime/explicit)

    ActionField& operator=(const ActionField&) = delete;
    void operator=(const std::shared_ptr<Action<T>>& action);
    void operator=(const char* named_action);
    void operator=(const std::string& named_action);
  };

  class NargsField {
   public:
    NargsField() {}
    NargsField(int value);        // NOLINT(runtime/explicit)
    NargsField(const char* str);  // NOLINT(runtime/explicit)
    NargsField(char c);           // NOLINT(runtime/explicit)

    NargsField& operator=(const NargsField&) = delete;
    void operator=(int value);
    void operator=(const char* str);
    void operator=(char c);
  };

  class ConstField {
   public:
    ConstField() {}
    ConstField(const T& value);  // NOLINT(runtime/explicit)

    ConstField& operator=(const ConstField&) = delete;
    void operator=(const T& value);
  };

  class DefaultField {
   public:
    DefaultField() {}
    DefaultField(const T& value);  // NOLINT(runtime/explicit)

    DefaultField& operator=(const DefaultField&) = delete;
    void operator=(const T& value);
  };

  class ChoicesField {
   public:
    ChoicesField() {}
    ChoicesField(
        const std::initializer_list<T>& choices);  // NOLINT(runtime/explicit)

    ChoicesField& operator=(const ChoicesField&) = delete;
    void operator=(const std::initializer_list<T>& choices);
  };

  class DestinationField {
   public:
    DestinationField() {}
    DestinationField(T* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::list<T, Allocator>* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::vector<T, Allocator>* destination);  // NOLINT(runtime/explicit)

    DestinationField& operator=(const DestinationField&) = delete;
    void operator=(T* destination);
    template <class Allocator>
    void operator=(std::list<T, Allocator>* destination);
    template <class Allocator>
    void operator=(std::vector<T, Allocator>* destination);
  };

  class RequiredField {
   public:
    RequiredField() {}
    RequiredField(bool value);  // NOLINT(runtime/explicit)

    RequiredField& operator=(const RequiredField&) = delete;
    void operator=(bool value);
  };

  class HelpField {
   public:
    HelpField() {}
    HelpField(const std::string& value);  // NOLINT(runtime/explicit)
    HelpField(const char* value);         // NOLINT(runtime/explicit)

    HelpField& operator=(const HelpField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  class MetavarField {
   public:
    MetavarField() {}
    MetavarField(const std::string& value);  // NOLINT(runtime/explicit)
    MetavarField(const char* value);         // NOLINT(runtime/explicit)

    MetavarField& operator=(const MetavarField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  ActionField action;
  NargsField nargs;
  ConstField const_;
  DefaultField default_;
  ChoicesField choices;
  DestinationField dest;
  RequiredField required;
  HelpField help;
  MetavarField metavar;
};

template <>
class KWargs<bool> {
 public:
  class ActionField : public std::shared_ptr<Action<bool>> {
   public:
    ActionField();
    ActionField(const std::shared_ptr<Action<bool>>&
                    action);                       // NOLINT(runtime/explicit)
    ActionField(const char* named_action);         // NOLINT(runtime/explicit)
    ActionField(const std::string& named_action);  // NOLINT(runtime/explicit)

    ActionField& operator=(const ActionField&) = delete;
    void operator=(const std::shared_ptr<Action<bool>>& action);
    void operator=(const char* named_action);
    void operator=(const std::string& named_action);
  };

  class NargsField {
   public:
    NargsField() {}
    NargsField(int value);        // NOLINT(runtime/explicit)
    NargsField(const char* str);  // NOLINT(runtime/explicit)
    NargsField(char c);           // NOLINT(runtime/explicit)

    NargsField& operator=(const NargsField&) = delete;
    void operator=(int value);
    void operator=(const char* str);
    void operator=(char c);
  };

  class ConstField {
   public:
    ConstField() {}
    ConstField(bool value);  // NOLINT(runtime/explicit)

    ConstField& operator=(const ConstField&) = delete;
    void operator=(bool value);
  };

  class DefaultField {
   public:
    DefaultField() {}
    DefaultField(bool value);  // NOLINT(runtime/explicit)

    DefaultField& operator=(const DefaultField&) = delete;
    void operator=(bool value);
  };

  class DestinationField {
   public:
    DestinationField() {}
    DestinationField(bool* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::list<bool, Allocator>* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::vector<bool, Allocator>* destination);  // NOLINT(runtime/explicit)

    DestinationField& operator=(const DestinationField&) = delete;
    void operator=(bool* destination);
    template <class Allocator>
    void operator=(std::list<bool, Allocator>* destination);
    template <class Allocator>
    void operator=(std::vector<bool, Allocator>* destination);
  };

  class RequiredField {
   public:
    RequiredField() {}
    RequiredField(bool value);  // NOLINT(runtime/explicit)

    RequiredField& operator=(const RequiredField&) = delete;
    void operator=(bool value);
  };

  class HelpField {
   public:
    HelpField() {}
    HelpField(const std::string& value);  // NOLINT(runtime/explicit)
    HelpField(const char* value);         // NOLINT(runtime/explicit)

    HelpField& operator=(const HelpField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  class MetavarField {
   public:
    MetavarField() {}
    MetavarField(const std::string& value);  // NOLINT(runtime/explicit)
    MetavarField(const char* value);         // NOLINT(runtime/explicit)

    MetavarField& operator=(const MetavarField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  ActionField action;
  NargsField nargs;
  ConstField const_;
  DefaultField default_;
  DestinationField dest;
  RequiredField required;
  HelpField help;
  MetavarField metavar;
};

template <>
class KWargs<void> {
 public:
  class ActionField : public std::shared_ptr<Action<void>> {
   public:
    ActionField() {}
    ActionField(const std::shared_ptr<Action<void>>&
                    action);                       // NOLINT(runtime/explicit)
    ActionField(const char* named_action);         // NOLINT(runtime/explicit)
    ActionField(const std::string& named_action);  // NOLINT(runtime/explicit)

    ActionField& operator=(const ActionField&) = delete;
    void operator=(const std::shared_ptr<Action<void>>& action);
    void operator=(const char* named_action);
    void operator=(const std::string& named_action);
  };

  class HelpField {
   public:
    HelpField() {}
    HelpField(const std::string& value);  // NOLINT(runtime/explicit)
    HelpField(const char* value);         // NOLINT(runtime/explicit)

    HelpField& operator=(const HelpField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  class MetavarField {
   public:
    MetavarField() {}
    MetavarField(const std::string& value);  // NOLINT(runtime/explicit)
    MetavarField(const char* value);         // NOLINT(runtime/explicit)

    MetavarField& operator=(const MetavarField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  ActionField action;
  HelpField help;
  MetavarField metavar;
};

}  // namespace argue
