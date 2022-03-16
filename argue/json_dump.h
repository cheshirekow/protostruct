#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cassert>
#include <cstdint>
#include <map>
#include <memory>

#include "tangent/util/array_stack.h"

namespace argue {

// ----------------------------------------------------------------------------
//    Dumper API
// ----------------------------------------------------------------------------

// Options for serialization
struct SerializeOpts {
  size_t indent;          //< Number of spaces to use for indent
  char separators[2][3];  //< map and list separators, i.e. ":" and ","
};

const SerializeOpts kDefaultOpts = {2, {":", ","}};

// Enumeration for the different semantic events that we might notify the
// dumper about
struct DumpEvent {
  enum TypeNo {
    OBJECT_BEGIN,
    OBJECT_KEY,
    OBJECT_VALUE,
    OBJECT_END,
    LIST_BEGIN,
    LIST_END,
    LIST_VALUE,
    INVALID,
  };

  TypeNo typeno;
  static const char* to_string(TypeNo value);
};

// Interface for JSON output formatters. Can also be implemented for a kind of
// poor-mans introspection on your serializable types.
class Dumper {
 public:
  explicit Dumper(const SerializeOpts& opts);

  // Push an event notification (e.g. semantic boundaries) to the output
  virtual void dump_event(DumpEvent::TypeNo eventno) = 0;

  // Dump a field whose value is a list stored by a standard container.
  // Pushes event notifications for the field key and value followed by the
  // actual string and value.
  template <class Container>
  int dump_container_field(const std::string& key, const Container& value);

  // Dump a field given it's name (as a string) and prep for the value. Pushes
  // event notifications for the field key and value followed by the actual
  // string. The next dump should be a value, object, or list.
  int dump_field_prefix(const std::string& key);

  // Dump a field given it's name (as a string) and value. Pushes event
  // notifications for the field key and value followed by the actual
  // string and value.
  template <class T>
  int dump_field(const std::string& key, const T& value);

  template <class T>
  int dump_item(const T& value);

  virtual void dump_primitive(uint8_t value) = 0;
  virtual void dump_primitive(uint16_t value) = 0;
  virtual void dump_primitive(uint32_t value) = 0;
  virtual void dump_primitive(uint64_t value) = 0;
  virtual void dump_primitive(int8_t value) = 0;
  virtual void dump_primitive(int16_t value) = 0;
  virtual void dump_primitive(int32_t value) = 0;
  virtual void dump_primitive(int64_t value) = 0;
  virtual void dump_primtiive(float value) = 0;
  virtual void dump_primitive(double value) = 0;
  virtual void dump_primitive(bool value) = 0;
  virtual void dump_primitive(std::nullptr_t nullval) = 0;
  virtual void dump_primitive(const std::string& strval) = 0;
  virtual void dump_primitive(const char* strval) = 0;

 protected:
  SerializeOpts opts_;  //< options for how to format the output
};

// Indicates the type of scope guard
enum GuardType {
  GUARD_OBJECT,
  GUARD_LIST,
};

// Scope guard for begin/end pairs
class DumpGuard {
 public:
  // Push the begin-event notification to the dumper
  DumpGuard(Dumper* dumper, GuardType type);

  // Push the end-event notification to the dumper
  ~DumpGuard();

 private:
  Dumper* dumper_;
  GuardType type_;
};

// Indicates the type of "aggregate" which is currently open at each level in
// the current stack of dump calls.
struct DumpStack {
  enum TypeNo {
    OBJECT,
    LIST,
    FIELD,
  };

  TypeNo type;
  uint32_t count;
};

// Dumper implementation which writes output to a std::ostream using stream
// operators.
class StreamDumper : public Dumper {
 public:
  // NOLINTNEXTLINE
  StreamDumper(std::ostream* ostream, const SerializeOpts& opts = kDefaultOpts);
  virtual ~StreamDumper() {}

  void dump_event(DumpEvent::TypeNo eventno) override;
  void dump_primitive(uint8_t value) override;
  void dump_primitive(uint16_t value) override;
  void dump_primitive(uint32_t value) override;
  void dump_primitive(uint64_t value) override;
  void dump_primitive(int8_t value) override;
  void dump_primitive(int16_t value) override;
  void dump_primitive(int32_t value) override;
  void dump_primitive(int64_t value) override;
  void dump_primtiive(float value) override;
  void dump_primitive(double value) override;
  void dump_primitive(bool value) override;
  void dump_primitive(std::nullptr_t nullval) override;
  void dump_primitive(const std::string& strval) override;
  void dump_primitive(const char* strval) override;

 private:
  std::ostream* ostream_;
  tangent::ArrayStack<DumpStack, 20> dump_stack_;
};

}  // namespace argue

//
//
//
// ============================================================================
//     Template Implementations
// ============================================================================
//
//
//

namespace argue {

// -----------------------------------------------------------------------------
//    class Dumper
// -----------------------------------------------------------------------------
template <class T>
int Dumper::dump_field(const std::string& key, const T& value) {
  int result = dump_field_prefix(key);
  this->dump_primitive(value);
  return result;
}

template <class T>
int Dumper::dump_item(const T& value) {
  this->dump_event(DumpEvent::LIST_VALUE);
  this->dump_primitive(value, this);
  return 0;
}

template <class Container>
int Dumper::dump_container_field(const std::string& key,
                                 const Container& value) {
  this->dump_event(DumpEvent::OBJECT_KEY);
  this->dump_primitive(key);
  this->dump_event(DumpEvent::OBJECT_VALUE);
  this->dump_primitive(value);
  return 0;
}

}  // namespace argue
