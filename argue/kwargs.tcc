#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/exception.h"
#include "argue/kwargs.h"
#include "argue/parse.h"
#include "tangent/util/container_of.h"

#include "argue/action.tcc"

namespace argue {

template <typename T>
KWargs<T>::ActionField::ActionField()
    : std::shared_ptr<Action<T>>(std::make_shared<StoreValue<T>>()) {}

template <typename T>
KWargs<T>::ActionField::ActionField(const std::shared_ptr<Action<T>>& action)
    : std::shared_ptr<Action<T>>(action) {}

template <typename T>
KWargs<T>::ActionField::ActionField(const char* named_action) {
  (*this) = named_action;
}

template <typename T>
void KWargs<T>::ActionField::operator=(
    const std::shared_ptr<Action<T>>& action) {
  (*static_cast<std::shared_ptr<Action<T>>*>(this)) = action;
}

template <typename T>
void KWargs<T>::ActionField::operator=(const char* named_action) {
  std::shared_ptr<Action<T>> action;
  if (strcmp(named_action, "store") == 0) {
    action = std::make_shared<StoreValue<T>>();
  } else if (strcmp(named_action, "store_const") == 0) {
    action = std::make_shared<StoreConst<T>>();
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false) << fmt::format(
        "invalid action={} for type={}", named_action, type_string<T>());
  }
  this->swap(action);
}

template <typename T>
void KWargs<T>::ActionField::operator=(const std::string& named_action) {
  (*this) = named_action.c_str();
}

template <typename T>
KWargs<T>::NargsField::NargsField(int value) {
  (*this) = value;
}

template <typename T>
KWargs<T>::NargsField::NargsField(const char* str) {
  if (str && str[0] != '\0') {
    (*this) = str;
  }
}

template <typename T>
KWargs<T>::NargsField::NargsField(char c) {
  (*this) = c;
}

template <typename T>
void KWargs<T>::NargsField::operator=(int value) {
  container_of(this, &KWargs<T>::nargs)->action->set_nargs(value);
}

template <typename T>
void KWargs<T>::NargsField::operator=(const char* str) {
  int value = string_to_nargs(str);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", str);
  (*this) = value;
}

template <typename T>
void KWargs<T>::NargsField::operator=(char c) {
  int value = string_to_nargs(c);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", c);
  (*this) = value;
}

template <typename T>
KWargs<T>::ConstField::ConstField(const T& value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::ConstField::operator=(const T& value) {
  container_of(this, &KWargs<T>::const_)->action->set_const(value);
}

template <typename T>
KWargs<T>::DefaultField::DefaultField(const T& value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::DefaultField::operator=(const T& value) {
  container_of(this, &KWargs<T>::default_)->action->set_default(value);
}

template <typename T>
KWargs<T>::ChoicesField::ChoicesField(const std::initializer_list<T>& choices) {
  (*this) = choices;
}

template <typename T>
void KWargs<T>::ChoicesField::operator=(
    const std::initializer_list<T>& choices) {
  std::vector<T> temp = choices;
  container_of(this, &KWargs<T>::choices)->action->set_choices(std::move(temp));
}

template <typename T>
KWargs<T>::DestinationField::DestinationField(T* destination) {
  (*this) = destination;
}

template <typename T>
template <class Allocator>
KWargs<T>::DestinationField::DestinationField(
    std::list<T, Allocator>* destination) {
  (*this) = destination;
}

template <typename T>
template <class Allocator>
KWargs<T>::DestinationField::DestinationField(
    std::vector<T, Allocator>* destination) {
  (*this) = destination;
}

template <typename T>
void KWargs<T>::DestinationField::operator=(T* destination) {
  container_of(this, &KWargs<T>::dest)->action->set_destination(destination);
}

template <typename T>
template <class Allocator>
void KWargs<T>::DestinationField::operator=(
    std::vector<T, Allocator>* destination) {
  container_of(this, &KWargs<T>::dest)->action->set_destination(destination);
}

template <typename T>
template <class Allocator>
void KWargs<T>::DestinationField::operator=(
    std::list<T, Allocator>* destination) {
  container_of(this, &KWargs<T>::dest)->action->set_destination(destination);
}

template <typename T>
KWargs<T>::RequiredField::RequiredField(bool value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::RequiredField::operator=(bool value) {
  container_of(this, &KWargs<T>::required)->action->set_required(value);
}

template <typename T>
KWargs<T>::HelpField::HelpField(const std::string& value) {
  (*this) = value;
}
template <typename T>
KWargs<T>::HelpField::HelpField(const char* value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::HelpField::operator=(const std::string& value) {
  container_of(this, &KWargs<T>::help)->action->set_help(value);
}
template <typename T>
void KWargs<T>::HelpField::operator=(const char* value) {
  container_of(this, &KWargs<T>::help)->action->set_help(value);
}

template <typename T>
KWargs<T>::MetavarField::MetavarField(const std::string& value) {
  (*this) = value;
}
template <typename T>
KWargs<T>::MetavarField::MetavarField(const char* value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::MetavarField::operator=(const std::string& value) {
  container_of(this, &KWargs<T>::metavar)->action->set_metavar(value);
}
template <typename T>
void KWargs<T>::MetavarField::operator=(const char* value) {
  container_of(this, &KWargs<T>::metavar)->action->set_metavar(value);
}

template <class Allocator>
KWargs<bool>::DestinationField::DestinationField(
    std::list<bool, Allocator>* destination) {
  (*this) = destination;
}

template <class Allocator>
KWargs<bool>::DestinationField::DestinationField(
    std::vector<bool, Allocator>* destination) {
  (*this) = destination;
}

template <class Allocator>
void KWargs<bool>::DestinationField::operator=(
    std::list<bool, Allocator>* destination) {
  std::shared_ptr<StorageModel<bool>> model =
      std::make_shared<ListModel<bool, Allocator>>(destination);
  container_of(this, &KWargs<bool>::dest)->action->set_destination(destination);
}

template <class Allocator>
void KWargs<bool>::DestinationField::operator=(
    std::vector<bool, Allocator>* destination) {
  std::shared_ptr<StorageModel<bool>> model =
      std::make_shared<VectorModel<bool, Allocator>>(destination);
  container_of(this, &KWargs<bool>::dest)->action->set_destination(destination);
}

}  // namespace argue
