// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/kwargs.h"

#include <cstdint>

#include "argue/exception.h"
#include "argue/parse.h"
#include "tangent/util/container_of.h"

#include "argue/action.tcc"
#include "argue/kwargs.tcc"

namespace argue {

// =============================================================================
//                              KWargs
// =============================================================================

KWargs<bool>::ActionField::ActionField()
    : std::shared_ptr<Action<bool>>(std::make_shared<StoreValue<bool>>()) {}

KWargs<bool>::ActionField::ActionField(
    const std::shared_ptr<Action<bool>>& action)
    : std::shared_ptr<Action<bool>>(action) {}

KWargs<bool>::ActionField::ActionField(const char* named_action) {
  (*this) = named_action;
}

void KWargs<bool>::ActionField::operator=(
    const std::shared_ptr<Action<bool>>& action) {
  (*static_cast<std::shared_ptr<Action<bool>>*>(this)) = action;
}

void KWargs<bool>::ActionField::operator=(const char* named_action) {
  std::shared_ptr<Action<bool>> action;
  if (strcmp(named_action, "store") == 0) {
    action = std::make_shared<StoreValue<bool>>();
  } else if (strcmp(named_action, "store_const") == 0) {
    action = std::make_shared<StoreConst<bool>>();
  } else if (strcmp(named_action, "store_true") == 0) {
    action = std::make_shared<StoreConst<bool>>();
    action->set_default(false);
    action->set_const(true);
  } else if (strcmp(named_action, "store_false") == 0) {
    action = std::make_shared<StoreConst<bool>>();
    action->set_default(false);
    action->set_const(true);
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false)
        << fmt::format("unrecognized action={}", named_action);
  }
  this->swap(action);
}

KWargs<bool>::NargsField::NargsField(int value) {
  (*this) = value;
}

KWargs<bool>::NargsField::NargsField(const char* str) {
  if (str && str[0] != '\0') {
    (*this) = str;
  }
}

void KWargs<bool>::NargsField::operator=(int value) {
  // TODO(josh): implement promotion
  container_of(this, &KWargs<bool>::nargs)->action->set_nargs(value);
}

void KWargs<bool>::NargsField::operator=(const char* str) {
  int value = string_to_nargs(str);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", str);

  container_of(this, &KWargs<bool>::nargs)->action->set_nargs(value);
}

KWargs<bool>::ConstField::ConstField(bool value) {
  (*this) = value;
}

void KWargs<bool>::ConstField::operator=(bool value) {
  container_of(this, &KWargs<bool>::const_)->action->set_const(value);
}

KWargs<bool>::DefaultField::DefaultField(bool value) {
  (*this) = value;
}

void KWargs<bool>::DefaultField::operator=(bool value) {
  container_of(this, &KWargs<bool>::default_)->action->set_default(value);
}

KWargs<bool>::DestinationField::DestinationField(bool* destination) {
  (*this) = destination;
}

void KWargs<bool>::DestinationField::operator=(bool* destination) {
  container_of(this, &KWargs<bool>::dest)->action->set_destination(destination);
}

KWargs<bool>::RequiredField::RequiredField(bool value) {
  (*this) = value;
}

void KWargs<bool>::RequiredField::operator=(bool value) {
  container_of(this, &KWargs<bool>::required)->action->set_required(value);
}

KWargs<bool>::HelpField::HelpField(const std::string& value) {
  (*this) = value;
}

KWargs<bool>::HelpField::HelpField(const char* value) {
  (*this) = value;
}

void KWargs<bool>::HelpField::operator=(const std::string& value) {
  container_of(this, &KWargs<bool>::help)->action->set_help(value);
}

void KWargs<bool>::HelpField::operator=(const char* value) {
  container_of(this, &KWargs<bool>::help)->action->set_help(value);
}

KWargs<bool>::MetavarField::MetavarField(const std::string& value) {
  (*this) = value;
}

KWargs<bool>::MetavarField::MetavarField(const char* value) {
  (*this) = value;
}

void KWargs<bool>::MetavarField::operator=(const std::string& value) {
  container_of(this, &KWargs<bool>::metavar)->action->set_metavar(value);
}

void KWargs<bool>::MetavarField::operator=(const char* value) {
  container_of(this, &KWargs<bool>::metavar)->action->set_metavar(value);
}

KWargs<void>::ActionField::ActionField(
    const std::shared_ptr<Action<void>>& action)
    : std::shared_ptr<Action<void>>(action) {}

KWargs<void>::ActionField::ActionField(const char* named_action) {
  (*this) = named_action;
}

KWargs<void>::ActionField::ActionField(const std::string& named_action) {
  (*this) = named_action;
}

void KWargs<void>::ActionField::operator=(
    const std::shared_ptr<Action<void>>& action) {
  (*static_cast<std::shared_ptr<Action<void>>*>(this)) = action;
}

void KWargs<void>::ActionField::operator=(const char* named_action) {
  std::shared_ptr<Action<void>> action;
  if (strcmp(named_action, "help") == 0) {
    action = std::make_shared<Help>();
  } else if (strcmp(named_action, "version") == 0) {
    action = std::make_shared<Version>();
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false)
        << fmt::format("unrecognized action={}", named_action);
  }
  this->swap(action);
}

void KWargs<void>::ActionField::operator=(const std::string& named_action) {
  (*this) = named_action.c_str();
}

KWargs<void>::HelpField::HelpField(const std::string& value) {
  (*this) = value;
}

KWargs<void>::HelpField::HelpField(const char* value) {
  (*this) = value;
}

void KWargs<void>::HelpField::operator=(const std::string& value) {
  container_of(this, &KWargs<void>::help)->action->set_help(value);
}

void KWargs<void>::HelpField::operator=(const char* value) {
  container_of(this, &KWargs<void>::help)->action->set_help(value);
}

KWargs<void>::MetavarField::MetavarField(const std::string& value) {
  (*this) = value;
}

KWargs<void>::MetavarField::MetavarField(const char* value) {
  (*this) = value;
}

void KWargs<void>::MetavarField::operator=(const std::string& value) {
  container_of(this, &KWargs<void>::metavar)->action->set_metavar(value);
}

void KWargs<void>::MetavarField::operator=(const char* value) {
  container_of(this, &KWargs<void>::metavar)->action->set_metavar(value);
}

// NOTE(josh): work around apparent bug in gcc 5.5??
template class KWargs<int16_t>;
template class KWargs<int32_t>;
template class KWargs<int64_t>;
template class KWargs<uint16_t>;
template class KWargs<uint32_t>;
template class KWargs<uint64_t>;
template class KWargs<std::string>;

}  // namespace argue
