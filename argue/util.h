#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <list>
#include <vector>

namespace argue {

// =============================================================================
//                                 Utilities
// =============================================================================

// Create a string formed by repeating `bit` for `n` times.
std::string repeat(const std::string bit, int n);

// Wrap the given text to the specified line length
std::string wrap(const std::string text, size_t line_length = 80);

// Return true if the query is a valid choice (i.e. matches with equality an
// element of the vector)
template <typename T>
bool has_choice(const std::vector<T>& choices, const T& query);

// Return a vector of the keys of an associative container (i.e. std::map)
template <typename Container>
std::vector<typename Container::key_type> keys(const Container& container);

// Compute the sum of all the elements of a container
template <typename Container>
typename Container::value_type container_sum(const Container& container);

// Template metaprogram evaluates to the value type of a container or the
// input type of a non container. The default template is for scalar types
// and just evaluates to the input type.
template <typename Scalar>
class ElementType {
 public:
  typedef Scalar value;
};

// Specialization for std::list, evaluates to the value type
template <typename T, class Allocator>
class ElementType<std::list<T, Allocator>> {
 public:
  typedef T value;
};

// Specialization for std::vector, evaluates to the value type
template <typename T, class Allocator>
class ElementType<std::vector<T, Allocator>> {
 public:
  typedef T value;
};

}  // namespace argue

//
//
//
// =============================================================================
//                       Template Implementations
// =============================================================================
//
//
//

namespace argue {

template <typename T>
bool has_choice(const std::vector<T>& choices, const T& query) {
  for (const T& choice : choices) {
    if (query == choice) {
      return true;
    }
  }
  return false;
}

template <typename Container>
std::vector<typename Container::key_type> keys(const Container& container) {
  std::vector<typename Container::key_type> out;
  out.reserve(container.size());
  for (auto& pair : container) {
    out.emplace_back(pair.first);
  }
  return out;
}

template <typename Container>
typename Container::value_type container_sum(const Container& container) {
  typename Container::value_type result = 0;
  for (auto x : container) {
    result += x;
  }
  return result;
}

}  // namespace argue
