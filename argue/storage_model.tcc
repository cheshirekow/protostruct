#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/storage_model.h"

#include "argue/exception.h"
#include "tangent/util/type_string.h"

namespace argue {

template <typename T, class Allocator>
ListModel<T, Allocator>::ListModel(std::list<T, Allocator>* dest)
    : dest_(dest) {
  this->type_name_ = type_string<ListModel<T, Allocator>>();
}

template <typename T, class Allocator>
void ListModel<T, Allocator>::init(size_t /*capacity_hint*/) {
  dest_->clear();
}

template <typename T, class Allocator>
void ListModel<T, Allocator>::append(const T& value) {
  dest_->emplace_back(value);
}

template <typename T, class Allocator>
void ListModel<T, Allocator>::assign(const T& value) {
  ARGUE_THROW(CONFIG_ERROR) << "You can't use a ListModel in a scalar context";
}

template <typename T, class Allocator>
VectorModel<T, Allocator>::VectorModel(std::vector<T, Allocator>* dest)
    : dest_(dest) {
  this->type_name_ = type_string<VectorModel<T, Allocator>>();
}

template <typename T, class Allocator>
void VectorModel<T, Allocator>::init(size_t capacity_hint) {
  dest_->clear();
  dest_->reserve(capacity_hint);
}

template <typename T, class Allocator>
void VectorModel<T, Allocator>::append(const T& value) {
  dest_->emplace_back(value);
}

template <typename T, class Allocator>
void VectorModel<T, Allocator>::assign(const T& value) {
  ARGUE_THROW(CONFIG_ERROR)
      << "You can't use a VectorModel in a scalar context";
}

template <typename T>
ScalarModel<T>::ScalarModel(T* dest) : dest_{dest} {}

template <typename T>
void ScalarModel<T>::init(size_t capacity_hint) {
  ARGUE_THROW(CONFIG_ERROR) << "You can't use a ScalarModel in a list context";
}

template <typename T>
void ScalarModel<T>::append(const T& value) {
  ARGUE_THROW(CONFIG_ERROR) << "You can't use a ScalarModel in a list context";
}

template <typename T>
void ScalarModel<T>::assign(const T& value) {
  (*dest_) = value;
}

}  // namespace argue
