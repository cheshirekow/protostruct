// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/json_dump.h"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <string>

namespace argue {

/// Return true if the given character is a control code
static int32_t is_control_code(char code) {
  return ('\x00' <= code && code <= '\x1f');
}

struct CodePair {
  char key;
  char value;
};

/// Map control codes that have a JSON shortcode to their shortcode
static const struct CodePair kEscapeMap[] = {
    // clang-format off
    {'"', '"'},
    {'\\', '\\'},
    {'\b', 'b'},
    {'\f', 'f'},
    {'\n', 'n'},
    {'\r', 'r'},
    {'\t', 't'},
    {0, 0}
    // clang-format on
};

static const struct CodePair* map_find_key(const struct CodePair* map,
                                           char key) {
  for (; map->key; map++) {
    if (map->key == key) {
      return map;
    }
  }
  return NULL;
}

// adapted from: https://stackoverflow.com/a/33799784/141023
void escape(const std::string& value, std::ostream* out) {
  for (char cvalue : value) {
    const struct CodePair* pair = map_find_key(kEscapeMap, cvalue);
    if (pair) {
      (*out) << '\\';
      (*out) << pair->value;
    } else if (is_control_code(cvalue)) {
      (*out) << "\\u" << std::setfill('0') << std::setw(4) << std::hex
             << static_cast<int>(cvalue);
    } else {
      (*out) << cvalue;
    }
  }
}

// -----------------------------------------------------------------------------
//    class Dumper
// -----------------------------------------------------------------------------

Dumper::Dumper(const SerializeOpts& opts) : opts_(opts) {}

int Dumper::dump_field_prefix(const std::string& key) {
  this->dump_event(DumpEvent::OBJECT_KEY);
  this->dump_primitive(key);
  this->dump_event(DumpEvent::OBJECT_VALUE);
  return 0;
}

// -----------------------------------------------------------------------------
//    class DumpGuard
// -----------------------------------------------------------------------------

DumpGuard::DumpGuard(Dumper* dumper, GuardType type)
    : dumper_(dumper), type_(type) {
  switch (type_) {
    case GUARD_OBJECT:
      dumper_->dump_event(DumpEvent::OBJECT_BEGIN);
      break;
    case GUARD_LIST:
      dumper_->dump_event(DumpEvent::LIST_BEGIN);
      break;
  }
}

DumpGuard::~DumpGuard() {
  switch (type_) {
    case GUARD_OBJECT:
      dumper_->dump_event(DumpEvent::OBJECT_END);
      break;
    case GUARD_LIST:
      dumper_->dump_event(DumpEvent::LIST_END);
      break;
  }
}

// -----------------------------------------------------------------------------
//    class StreamDumper
// -----------------------------------------------------------------------------

StreamDumper::StreamDumper(std::ostream* ostream, const SerializeOpts& opts)
    : Dumper(opts), ostream_{ostream} {}

void StreamDumper::dump_event(DumpEvent::TypeNo eventno) {
  switch (eventno) {
    case DumpEvent::LIST_BEGIN:
      (*ostream_) << "[";
      if (opts_.indent) {
        (*ostream_) << "\n";
      }
      dump_stack_.push_back({DumpStack::LIST, 0});
      break;

    case DumpEvent::LIST_END:
      assert(dump_stack_.size());
      assert(dump_stack_.back().type == DumpStack::LIST);
      if (opts_.indent && dump_stack_.back().count) {
        (*ostream_) << "\n";
        for (size_t idx = 0; idx < (dump_stack_.size() - 1) * opts_.indent;
             idx++) {
          (*ostream_) << ' ';
        }
      }
      (*ostream_) << "]";
      dump_stack_.pop_back();
      break;

    case DumpEvent::OBJECT_BEGIN:
      (*ostream_) << "{";
      if (opts_.indent) {
        (*ostream_) << "\n";
      }
      dump_stack_.push_back({DumpStack::OBJECT, 0});
      break;

    case DumpEvent::OBJECT_END:
      assert(dump_stack_.size());
      assert(dump_stack_.back().type == DumpStack::OBJECT);
      if (opts_.indent && dump_stack_.back().count) {
        (*ostream_) << "\n";
        for (size_t idx = 0; idx < (dump_stack_.size() - 1) * opts_.indent;
             idx++) {
          (*ostream_) << ' ';
        }
      }
      (*ostream_) << "}";
      dump_stack_.pop_back();
      break;

    case DumpEvent::LIST_VALUE:
    case DumpEvent::OBJECT_KEY:
      // This is the start of a field in an object or a value in a list,
      // if it is not the first, then write out the "," separator
      if (dump_stack_.size() && dump_stack_.back().count) {
        (*ostream_) << opts_.separators[1];
        if (opts_.indent) {
          (*ostream_) << "\n";
        }
      }
      for (size_t idx = 0; idx < dump_stack_.size() * opts_.indent; idx++) {
        (*ostream_) << ' ';
      }

      // This is *not* the end of an aggregate, so whatever aggregate is at
      // the top of the stack is getting a new element
      dump_stack_.back().count += 1;
      break;

    case DumpEvent::OBJECT_VALUE:
      // This is the value for a field, following the field name, so we
      // should write out the ":" separator.
      (*ostream_) << opts_.separators[0];
      break;
    default:
      break;
  }
}

void StreamDumper::dump_primitive(uint8_t value) {
  (*ostream_) << static_cast<int32_t>(value);
}

void StreamDumper::dump_primitive(uint16_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(uint32_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(uint64_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(int8_t value) {
  (*ostream_) << static_cast<int32_t>(value);
}

void StreamDumper::dump_primitive(int16_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(int32_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(int64_t value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primtiive(float value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(double value) {
  (*ostream_) << value;
}

void StreamDumper::dump_primitive(bool value) {
  (*ostream_) << (value ? "true" : "false");
}

void StreamDumper::dump_primitive(std::nullptr_t nullval) {
  (*ostream_) << "null";
}

void StreamDumper::dump_primitive(const std::string& strval) {
  (*ostream_) << '\"';
  escape(strval, ostream_);
  (*ostream_) << '\"';
}

void StreamDumper::dump_primitive(const char* strval) {
  (*ostream_) << '\"';
  escape(strval, ostream_);
  (*ostream_) << '\"';
}

}  // namespace argue
