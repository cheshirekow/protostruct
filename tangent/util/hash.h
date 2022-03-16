#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdint>
#include <string>

namespace tangent {

namespace hash_detail {

constexpr char char_tolower(const char c) {
  return (c >= 'A' && c <= 'Z') ? c + -'A' + 'a' : c;
}

// constexpr string, used to implement tagging with strings
/*
 *  via Scott Schurr's C++ Now 2012 presentation
 */
class Tag {
 public:
  template <uint32_t N>
  // Construct from a string literal
  constexpr Tag(const char (&str)[N]) : ptr_(str), size_(N - 1) {}  // NOLINT
  constexpr Tag(const char* str, uint32_t len) : ptr_(str), size_(len) {}

  // return the character at index i
  constexpr char operator[](uint32_t i) const {
    // if we care about overflow
    // return n < size_ ? ptr_[i] : throw std::out_of_range("");
    return ptr_[i];
  }

  // return the number of characters in the string, including the terminal null
  constexpr uint32_t size() const {
    return size_;
  }

  // Donald Knuth's hash function
  // TODO(josh): validate or replace this hash function
  constexpr uint64_t hash(int i, uint64_t hashv) const {
    return static_cast<uint32_t>(i) == size()
               ? hashv
               : hash(i + 1, ((hashv << 5) ^ (hashv >> 27)) ^ ptr_[i]);
  }

  // return a hash of the string
  constexpr uint64_t hash() const {
    return hash(0, size());
  }

  // case-insensistive hash
  constexpr uint64_t ci_hash(int i, uint64_t hashv) const {
    return static_cast<uint32_t>(i) == size()
               ? hashv
               : ci_hash(i + 1, ((hashv << 5) ^ (hashv >> 27)) ^
                                    char_tolower(ptr_[i]));
  }

  constexpr uint64_t ci_hash() const {
    return ci_hash(0, size());
  }

  // Stripped-underscore, case-insensitive hash
  constexpr uint64_t suci_hash(int i, uint64_t hashv) const {
    return static_cast<uint32_t>(i) == size()
               ? hashv
               : (ptr_[i] == '_'
                      ? suci_hash(i + 1, hashv)
                      : suci_hash(i + 1, ((hashv << 5) ^ (hashv >> 27)) ^
                                             char_tolower(ptr_[i])));
  }

  constexpr uint64_t suci_hash() const {
    return suci_hash(0, size());
  }

 private:
  const char* const ptr_;
  const uint32_t size_;
};

}  // namespace hash_detail

// Return an unsigned 64bit hash of @p string
template <uint32_t N>
inline constexpr uint64_t hash(const char (&str)[N]) {
  return hash_detail::Tag(str).hash();
}

template <uint32_t N>
inline constexpr uint64_t ci_hash(const char (&str)[N]) {
  return hash_detail::Tag(str).ci_hash();
}

template <uint32_t N>
inline constexpr uint64_t suci_hash(const char (&str)[N]) {
  return hash_detail::Tag(str).suci_hash();
}

// Donald Knuth's hash function
// TODO(josh): validate or replace this hash function
// NOTE(josh): this is the same as above, but iterative instead of stack-based
inline uint64_t runtime_hash(const std::string& str) {
  uint64_t hash = str.size();
  for (size_t idx = 0; idx < str.size(); ++idx) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ str[idx];
  }
  return hash;
}

inline uint64_t runtime_ci_hash(const std::string& str) {
  uint64_t hash = str.size();
  for (size_t idx = 0; idx < str.size(); ++idx) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ hash_detail::char_tolower(str[idx]);
  }
  return hash;
}

inline uint64_t runtime_suci_hash(const std::string& str) {
  uint64_t hash = str.size();
  for (size_t idx = 0; idx < str.size(); ++idx) {
    if (str[idx] == '_') {
      continue;
    }
    hash = ((hash << 5) ^ (hash >> 27)) ^ hash_detail::char_tolower(str[idx]);
  }
  return hash;
}

}  // namespace tangent
