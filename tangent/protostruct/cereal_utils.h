#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdint>
#include <cereal/cereal.hpp>

namespace protostruct {

template <class ValueType, class SizeType>
struct FixedCapacityArray {
  ValueType* value_ptr;
  SizeType* size_ptr;
  uint32_t capacity;
};

template <class ValueType, class SizeType, uint32_t N>
FixedCapacityArray<ValueType, SizeType> cereal_array(
    ValueType (&value_ptr)[N], SizeType& size_ptr) {
  return {value_ptr, &size_ptr, N};
}

template<class T, size_t N>
constexpr size_t array_size(T (&)[N]){ return N; }

};  // namespace protostruct

template <class Archive, class ValueType, class SizeType>
void serialize(Archive& ar,
               protostruct::FixedCapacityArray<ValueType, SizeType>& array) {
  auto size_tag = cereal::make_size_tag(*(array.size_ptr));
  for (SizeType idx = 0; idx < array.capacity; idx++) {
    ar(array.value_ptr[idx]);
  }
}
