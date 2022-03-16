#pragma once
// Copyright 2021 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <list>
#include <sstream>
#include <vector>

#include "tangent/tjson/tjson.h"
#include "tangent/util/exception.h"
#include "tangent/util/fallthrough.h"

namespace tjson {

enum StackTypeNo { OBJECT = 0, LIST, FIELD };

struct StackElement {
  explicit StackElement(StackTypeNo typeno) : typeno{typeno}, child_count{0} {}

  StackTypeNo typeno;
  int child_count;
};

class OStream {
 public:
  OStream(std::ostream* out, const tjson_SerializeOpts& opts)
      : out_{out}, opts_{opts} {}

  void endif_field() {
    if (event_stack_.size() && event_stack_.back().typeno == FIELD) {
      event_stack_.pop_back();
    }
  }

  void begin(StackTypeNo typeno) {
    TANGENT_ASSERT(!event_stack_.size() || event_stack_.back().typeno != OBJECT)
        << "Attempt to write a JSON value (type " << typeno
        << ") but a fieldname is expected";
    if (event_stack_.size()) {
      preprocess_value(false);
    }
    event_stack_.emplace_back(typeno);
    switch (typeno) {
      case OBJECT:
        (*out_) << "{";
        break;
      case LIST:
        (*out_) << "[";
        break;
      default:
        break;
    }
  }

  void end(StackTypeNo typeno) {
    TANGENT_ASSERT(event_stack_.size())
        << "Attempt to close a JSON value of type " << typeno
        << " but there isn't anything open";

    TANGENT_ASSERT(event_stack_.back().typeno == typeno)
        << "Attempt to close a JSON value of type " << typeno
        << " but the current value is of type " << event_stack_.back().typeno;

    auto child_count = event_stack_.back().child_count;
    event_stack_.pop_back();
    if (child_count > 0 && opts_.indent) {
      (*out_) << "\n";
      write_indent(/*fudge=*/0);
    }

    switch (typeno) {
      case OBJECT:
        (*out_) << "}";
        break;
      case LIST:
        (*out_) << "]";
        break;
      default:
        break;
    }
    // we just finished writing an object or list which was the value of a
    // field, so we are also finished with that field
    endif_field();
    if (event_stack_.size()) {
      event_stack_.back().child_count++;
    } else {
      // this is the terminal of the root object, so let's add a newline to
      // the end of the "document".
      (*out_) << "\n";
    }
  }

  void write_indent(int fudge = 1) {
    for (size_t idx = 0; idx < opts_.indent * (event_stack_.size() + fudge);
         idx++) {
      (*out_) << ' ';
    }
  }

  void preprocess_value(bool is_string = false) {
    TANGENT_ASSERT(event_stack_.size())
        << "Emit value but there is no open object";
    switch (event_stack_.back().typeno) {
      case OBJECT:
        TANGENT_ASSERT(is_string) << "Emit value but expected a fieldname";
        // this is a field name
        TANGENT_FALLTHROUGH

      case LIST:
        // this is a string value in a list
        if (event_stack_.back().child_count > 0) {
          (*out_) << opts_.separators[1];
          if (opts_.indent) {
            (*out_) << "\n";
          } else {
            (*out_) << " ";
          }
        } else {
          if (opts_.indent) {
            (*out_) << "\n";
          }
        }
        write_indent();
        break;

      case FIELD:
        // this is a field value
        break;
    }
  }

  void write_string(const char* value) {
    preprocess_value(true);
    (*out_) << '"' << value << '"';

    if (event_stack_.back().typeno == OBJECT) {
      (*out_) << opts_.separators[0];
      event_stack_.emplace_back(FIELD);
    } else {
      endif_field();
      event_stack_.back().child_count++;
    }
  }

  void write_string(const std::string& value) {
    write_string(value.c_str());
  }

  template <class T>
  void write_numeric(T value) {
    preprocess_value();
    (*out_) << value;
    endif_field();
    event_stack_.back().child_count++;
  }

  void write_numeric(int8_t value) {
    write_numeric(static_cast<int>(value));
  }

  void write_numeric(uint8_t value) {
    write_numeric(static_cast<unsigned int>(value));
  }

  void write_boolean(bool value) {
    preprocess_value();
    if (value) {
      (*out_) << "true";
    } else {
      (*out_) << "false";
    }
    endif_field();
    event_stack_.back().child_count++;
  }

  void write_null() {
    preprocess_value();
    (*out_) << "null";
    endif_field();
  }

 protected:
  std::ostream* out_;
  tjson_SerializeOpts opts_;
  std::vector<StackElement> event_stack_;
};

class Guard {
 public:
  Guard(OStream* out, StackTypeNo typeno) : out_{out}, typeno_{typeno} {
    out->begin(typeno);
  }

  ~Guard() {
    out_->end(typeno_);
  }

 private:
  OStream* out_;
  StackTypeNo typeno_;
};

inline OStream& operator<<(OStream& out, const char* str) {
  out.write_string(str);
  return out;
}

inline OStream& operator<<(OStream& out, const std::string& str) {
  out.write_string(str);
  return out;
}

template <class T, class Allocator>
inline OStream& operator<<(OStream& out,
                           const std::vector<T, Allocator>& list) {
  tjson::Guard guard{&out, tjson::LIST};
  for (const auto& elem : list) {
    out << elem;
  }
  return out;
}

template <class T, class Allocator>
inline OStream& operator<<(OStream& out, const std::list<T, Allocator>& list) {
  tjson::Guard guard{&out, tjson::LIST};
  for (const auto& elem : list) {
    out << elem;
  }
  return out;
}

inline OStream& operator<<(OStream& out, uint8_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, uint16_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, uint32_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, uint64_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, int8_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, int16_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, int32_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, int64_t value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, bool value) {
  out.write_boolean(value);
  return out;
}

inline OStream& operator<<(OStream& out, double value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, float value) {
  out.write_numeric(value);
  return out;
}

inline OStream& operator<<(OStream& out, std::nullptr_t) {
  out.write_null();
  return out;
}

class OFStreamBase {
 public:
  explicit OFStreamBase(const std::string& filename) : real_strm_{filename} {}
  explicit OFStreamBase(const char* filename) : real_strm_{filename} {}

 protected:
  std::ofstream real_strm_{};
};

class OFStream : public OFStreamBase, public OStream {
 public:
  explicit OFStream(const std::string& filename,
                    const tjson_SerializeOpts& opts = tjson_DefaultOpts)
      : OFStreamBase{filename}, OStream{&real_strm_, opts} {}

  explicit OFStream(const char* filename,
                    const tjson_SerializeOpts& opts = tjson_DefaultOpts)
      : OFStreamBase{filename}, OStream{&real_strm_, opts} {};

  bool good() {
    return real_strm_.good();
  }
};

class OSStreamBase {
 public:
  OSStreamBase() : real_strm_{} {}

 protected:
  std::stringstream real_strm_{};
};

class OSStream : public OSStreamBase, public OStream {
 public:
  explicit OSStream(const tjson_SerializeOpts& opts = tjson_DefaultOpts)
      : OStream{&real_strm_, opts} {}

  std::string str() {
    return real_strm_.str();
  }
};

}  // namespace tjson
