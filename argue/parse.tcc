#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/parse.h"

namespace argue {

template <typename T>
int parse_signed(const std::string& str, T* value) {
  *value = 0;

  size_t idx = 0;
  T multiplier = 1;

  if (str[0] == '-') {
    multiplier = -std::pow(10, str.size() - 2);
    ++idx;
  } else {
    multiplier = std::pow(10, str.size() - 1);
  }

  for (; idx < str.size(); idx++) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      *value += multiplier * static_cast<T>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  return 0;
}

template <typename T>
int parse_unsigned(const std::string& str, T* value) {
  *value = 0;

  T multiplier = std::pow(10, str.size() - 1);
  for (size_t idx = 0; idx < str.size(); idx++) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      *value += multiplier * static_cast<T>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  return 0;
}

template <typename T>
int parse_float(const std::string& str, T* value) {
  *value = 0.0;

  size_t decimal_idx = str.find('.');
  if (decimal_idx == std::string::npos) {
    decimal_idx = str.size();
  }

  int64_t integral_part = 0;
  int64_t fractional_part = 0;
  int64_t denominator = 10;

  size_t idx = 0;
  int32_t multiplier = 1;

  if (str[0] == '-') {
    multiplier = -std::pow(10, decimal_idx - 2);
    ++idx;
  } else {
    multiplier = std::pow(10, decimal_idx - 1);
  }

  for (; idx < decimal_idx; ++idx) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      integral_part += multiplier * static_cast<uint64_t>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  if (decimal_idx == str.size()) {
    *value = static_cast<T>(integral_part);
    return 0;
  }

  denominator = std::pow(10, str.size() - decimal_idx);
  if (str[0] == '-') {
    multiplier = -denominator / 10;
  } else {
    multiplier = denominator / 10;
  }

  ++idx;
  for (; idx < str.size(); ++idx) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      fractional_part += multiplier * static_cast<uint64_t>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  *value = static_cast<T>(integral_part) +
           static_cast<T>(fractional_part) / static_cast<T>(denominator);
  return 0;
}

template <typename T>
int parse(const std::string& str, std::shared_ptr<T>* ptr) {
  return -1;
}

template <typename T, class Allocator>
int parse(const std::string& str, std::list<T, Allocator>* ptr) {
  return -1;
}

template <typename T, class Allocator>
int parse(const std::string& str, std::vector<T, Allocator>* ptr) {
  return -1;
}

}  // namespace argue
