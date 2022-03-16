#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <sstream>
#include <string>
#include <vector>

namespace stringutil {

// Return a std::string from a c-string, or the value "None" if the pointer
// is empty.
const std::string or_none(const char* value);

// Return a lowercase copy of the string
std::string to_lower(const std::string& value);

inline std::string tolower(const std::string& value) {
  return to_lower(value);
}

// Return an uppercase copy of the string
std::string to_upper(const std::string& value);

inline std::string toupper(const std::string& value) {
  return to_upper(value);
}

// Split a string at each occurance of a delimeter
std::vector<std::string> split(const std::string& str, char delim = ' ');

// Return true if `needle` is a prefix of `haystack'.
bool starts_with(const std::string& haystack, const std::string& needle,
                 bool case_sensitive = true);

inline bool startswith(const std::string& haystack, const std::string& needle,
                       bool case_sensitive = true) {
  return starts_with(haystack, needle, case_sensitive);
}

// Return true if `needle` is a suffix of `haystack`.
bool ends_with(const std::string& haystack, const std::string& needle,
               bool case_sensitive = true);

inline bool endswith(const std::string& haystack, const std::string& needle,
                     bool case_sensitive = true) {
  return ends_with(haystack, needle, case_sensitive);
}

// Create a string by joining the elements of the container using default
// stream formatting and the provided delimeter. Uses
// ``ostream& operator<<(ostream&, ...)`` to format each element
template <class Container>
inline std::string join(const Container& elems, const std::string& glue = ", ");

// Split a string at each occurance of a delimieter. Push each item to the
// output container.
template <typename Out>
void split(const std::string& s, char delim, Out result);

std::vector<std::string> split(const std::string& s, char delim);

// return a copy `full` with `prefix` removed from the front, if `full` starts
// with prefix
std::string pop_prefix(const std::string& full, const std::string& prefix,
                       bool case_sensitive = true);

std::string pop_suffix(const std::string& full, const std::string& suffix,
                       bool case_sensitive = true);

std::string slice(const std::string& str, int begin, int end);

bool contains(const std::string& haystack, char needle);
bool contains(const std::string& haystack, const std::string& needle);

std::string lstrip(const std::string& str,
                   const std::string& stripchars = " \t\n\r");

std::string rstrip(const std::string& str,
                   const std::string& stripchars = " \t\n\r");

std::string strip(const std::string& str,
                  const std::string& stripchars = " \t\n\r");

std::string replace(const std::string& str, const std::string& what,
                    const std::string& with);

}  // namespace stringutil

// -----------------------------------------------------------------------------
//                          Template Impls
// -----------------------------------------------------------------------------
namespace stringutil {

template <class Container>
inline std::string join(const Container& elems, const std::string& glue) {
  std::stringstream strm;
  auto iter = elems.begin();
  if (iter != elems.end()) {
    strm << *iter;
    ++iter;
  }
  for (; iter != elems.end(); ++iter) {
    strm << glue;
    strm << *iter;
  }
  return strm.str();
}

template <typename Out>
void split(const std::string& s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

}  // namespace stringutil
