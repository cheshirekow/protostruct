#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cmath>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace argue {

// =============================================================================
//                          String Parsing
// =============================================================================

// Parse a base-10 string as into a signed integer. Matches strings of the
// form `[-?]\d+`.
template <typename T>
int parse_signed(const std::string& str, T* value);

// Parse a base-10 string into an unsigned integer. Matches strings of the
// form `\d+`.
template <typename T>
int parse_unsigned(const std::string& str, T* value);

// Parse a real-number string into a floating point value. Matches strings
// of the form `[-?]\d+\.?\d*`
template <typename T>
int parse_float(const std::string& str, T* value);

int parse(const std::string& str, uint8_t* value);
int parse(const std::string& str, uint16_t* value);
int parse(const std::string& str, uint32_t* value);
int parse(const std::string& str, uint64_t* value);
int parse(const std::string& str, int8_t* value);
int parse(const std::string& str, int16_t* value);
int parse(const std::string& str, int32_t* value);
int parse(const std::string& str, int64_t* value);
int parse(const std::string& str, float* value);
int parse(const std::string& str, double* value);
int parse(const std::string& str, bool* value);
int parse(const std::string& str, std::string* value);

template <typename T>
int parse(const std::string& str, std::shared_ptr<T>* ptr);

template <typename T, class Allocator>
int parse(const std::string& str, std::list<T, Allocator>* ptr);

template <typename T, class Allocator>
int parse(const std::string& str, std::vector<T, Allocator>* ptr);

// Tokens in an argument list are one of these.
enum ArgType { SHORT_FLAG = 0, LONG_FLAG = 1, POSITIONAL = 2, SEPARATOR = 3 };

// Return the ArgType of a string token:
//  * SHORT_FLAG if it is of the form `-[^-]`
//  * LONG_FLAG if it is of the form `--.+`
//  * POSITIONAL otherwise
ArgType get_arg_type(const std::string arg);

// Sentinel integer values used to indicate special `nargs`.
enum SentinelNargs {
  REMAINDER = -7,      //< consume all the remaining arguments, regardless
                       // of whether or not they appear to be positional or
                       // flag, into a list of strings
  ZERO_NARGS = -6,     //< no arguments, only has meaning for `store_const`
                       //  style flags and things
  INVALID_NARGS = -5,  //< initial value, or sentinel output from
                       // parsing `nargs` strings
  ONE_OR_MORE = -4,    //< consume values until we reach a flag or run out of
                       //  arguments, but throw an error if we do not consume
                       //  at least one
  ZERO_OR_MORE = -3,   //< consume values until we reach a flag or run out of
                       //  arguments
  ZERO_OR_ONE = -2,    //< consume one value if available, so long as it is not
                       //  a flag
  EXACTLY_ONE = -1,    //< consume one value if available, so long as it is not
                       //  a flag. Throw an error if there are no arguments
                       //  remaining or the next argument is a flag
};

// Parse a string (i.e. '?', '*', '+') into a sentinel `nargs` value.
int string_to_nargs(char key);

// Parse a string (i.e. '?', '*', '+') into a sentinel `nargs` value.
int string_to_nargs(const char* str);

// Parse a string (i.e. '?', '*', '+') into a sentinel `nargs` value.
int string_to_nargs(const std::string& str);

}  // namespace argue
