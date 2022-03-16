// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/util/stringutil.h"

#include <algorithm>
#include <iterator>

namespace stringutil {

const std::string or_none(const char* value) {
  return value ? value : "<None>";
}

std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

std::string to_lower(const std::string& instr) {
  std::string outstr(instr.size(), '0');
  std::transform(instr.begin(), instr.end(), outstr.begin(), ::tolower);
  return outstr;
}

std::string to_upper(const std::string& instr) {
  std::string outstr(instr.size(), '0');
  std::transform(instr.begin(), instr.end(), outstr.begin(), ::toupper);
  return outstr;
}

bool starts_with(const std::string& haystack, const std::string& needle,
                 bool case_sensitive) {
  if (case_sensitive) {
    return haystack.substr(0, needle.size()) == needle;
  } else {
    return to_lower(haystack.substr(0, needle.size())) == to_lower(needle);
  }
}

bool ends_with(const std::string& haystack, const std::string& needle,
               bool case_sensitive) {
  if (haystack.size() < needle.size()) {
    return false;
  }

  if (case_sensitive) {
    return haystack.substr(haystack.size() - needle.size()) == needle;
  } else {
    return to_lower(haystack.substr(haystack.size() - needle.size())) ==
           to_lower(needle);
  }
}

std::string pop_prefix(const std::string& full, const std::string& prefix,
                       bool case_sensitive) {
  if (starts_with(full, prefix, case_sensitive)) {
    return full.substr(prefix.size());
  }
  return full;
}

std::string pop_suffix(const std::string& full, const std::string& suffix,
                       bool case_sensitive) {
  if (ends_with(full, suffix, case_sensitive)) {
    return full.substr(0, full.size() - suffix.size());
  }
  return full;
}

std::string slice(const std::string& str, int begin, int end) {
  if (begin < 0) {
    begin = str.size() + begin;
  }
  if (end < 0) {
    end = str.size() + begin;
  }
  if (end <= begin) {
    return "";
  }
  return str.substr(begin, end - begin);
}

bool contains(const std::string& haystack, char needle) {
  return haystack.find(needle) != std::string::npos;
}

bool contains(const std::string& haystack, const std::string& needle) {
  return haystack.find(needle) != std::string::npos;
}

std::string lstrip(const std::string& str, const std::string& stripchars) {
  size_t begin_idx = 0;
  while (begin_idx < str.size() && contains(stripchars, str[begin_idx])) {
    begin_idx++;
  }
  return str.substr(begin_idx);
}

std::string rstrip(const std::string& str, const std::string& stripchars) {
  size_t size = str.size();
  while (size > 0 && contains(stripchars, str[size - 1])) {
    size--;
  }
  return str.substr(0, size);
}

std::string strip(const std::string& str, const std::string& stripchars) {
  return lstrip(rstrip(str, stripchars), stripchars);
}

std::string replace(const std::string& str, const std::string& what,
                    const std::string& with) {
  std::string::size_type match_pos = str.find(what);
  if (match_pos == str.npos) {
    return str;
  }
  return str.substr(0, match_pos) + with +
         str.substr(match_pos + what.length());
}

}  // namespace stringutil
