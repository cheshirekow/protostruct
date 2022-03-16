// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/parse.h"
#include "tangent/util/stringutil.h"

#include "argue/parse.tcc"

namespace argue {

// =============================================================================
//                          String Parsing
// =============================================================================

int parse(const std::string& str, uint8_t* value) {
  return parse_unsigned(str, value);
}

int parse(const std::string& str, uint16_t* value) {
  return parse_unsigned(str, value);
}

int parse(const std::string& str, uint32_t* value) {
  return parse_unsigned(str, value);
}

int parse(const std::string& str, uint64_t* value) {
  return parse_unsigned(str, value);
}

int parse(const std::string& str, int8_t* value) {
  return parse_signed(str, value);
}

int parse(const std::string& str, int16_t* value) {
  return parse_signed(str, value);
}

int parse(const std::string& str, int32_t* value) {
  return parse_signed(str, value);
}

int parse(const std::string& str, int64_t* value) {
  return parse_signed(str, value);
}

int parse(const std::string& str, float* value) {
  return parse_float(str, value);
}

int parse(const std::string& str, double* value) {
  return parse_float(str, value);
}

int parse(const std::string& str, bool* value) {
  std::string lower = stringutil::to_lower(str);
  if (lower == "true" || lower == "t" || lower == "yes" || lower == "y" ||
      lower == "on" || lower == "1") {
    *value = true;
    return 0;
  } else if (lower == "false" || lower == "f" || lower == "no" ||
             lower == "n" || lower == "off" || lower == "0") {
    *value = false;
    return 0;
  } else {
    return -1;
  }
}

int parse(const std::string& str, std::string* value) {
  *value = str;
  return 0;
}

int string_to_nargs(char str) {
  if (str == '+') {
    return ONE_OR_MORE;
  } else if (str == '*') {
    return ZERO_OR_MORE;
  } else if (str == '?') {
    return ZERO_OR_ONE;
  }
  return INVALID_NARGS;
}

int string_to_nargs(const char* str) {
  return string_to_nargs(str[0]);
}

int string_to_nargs(const std::string& str) {
  return string_to_nargs(str[0]);
}

ArgType get_arg_type(const std::string arg) {
  if (arg.size() > 1 && arg[0] == '-') {
    if (arg.size() == 2 && arg[1] == '-') {
      return SEPARATOR;
    }
    if (arg.size() > 2 && arg[1] == '-') {
      return LONG_FLAG;
    }
    if (arg[1] != '-') {
      return SHORT_FLAG;
    }

    return POSITIONAL;
  }

  return POSITIONAL;
}

}  // namespace argue
