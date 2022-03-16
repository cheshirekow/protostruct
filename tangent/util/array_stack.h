#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <array>
#include <type_traits>
#include <utility>

namespace tangent {
namespace fixed_vector {}  // namespace fixed_vector

template <typename T, int N>
class ArrayStack {
 public:
  using value_type = T;
  using iterator = T*;
  using const_iterator = const typename std::remove_const<T>::type*;

  ArrayStack() {
    begin_ = reinterpret_cast<T*>(&storage_[0]);
    end_ = begin_;
  }

  ~ArrayStack() {}

  iterator begin() {
    return begin_;
  }

  iterator end() {
    return end_;
  }

  T& back() {
    return end_[-1];
  }

  const T& back() const {
    return end_[-1];
  }

  const_iterator begin() const {
    return begin_;
  }

  const_iterator end() const {
    return end_;
  }

  void push_back(const T& value) {
    new (end_) T{value};
    end_++;
  }

  void push_back(const T&& value) {
    new (end_) T{value};
    end_++;
  }

  T get_pop_back() {
    T ret = *end_;
    end_--;
    return ret;
  }

  void pop_back() {
    end_--;
  }

  template <class... Args>
  void emplace_back(Args&&... args) {
    new (end_) T{std::forward<Args>(args)...};
    end_++;
  }

  size_t size() const {
    return (end_ - begin_);
  }

  size_t capacity() const {
    return N;
  }

  template <typename NumericT>
  T& operator[](NumericT idx) {
    if (idx >= 0) {
      return begin_[idx];
    } else {
      return end_[idx];
    }
  }

  template <typename NumericT>
  const T& operator[](NumericT idx) const {
    if (idx >= 0) {
      return begin_[idx];
    } else {
      return end_[idx];
    }
  }

 protected:
  alignas(sizeof(T)) char storage_[N * sizeof(T)];
  T* begin_{nullptr};
  T* end_{nullptr};
};

}  // namespace tangent
