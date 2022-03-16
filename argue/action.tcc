#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/action.h"
#include "argue/exception.h"
#include "argue/parse.h"
#include "argue/util.h"
#include "tangent/util/stringutil.h"

#include "argue/storage_model.tcc"

namespace argue {

template <typename T>
Action<T>::Action() : ActionBase{} {
  this->type_name_ = type_string<T>();
}

template <typename T>
void Action<T>::set_const(const T& value) {
  ARGUE_THROW(CONFIG_ERROR) << "const= is only valid for StoreConst actions";
}

template <typename T>
void Action<T>::set_default(const T& value) {
  default_.clear();
  default_.push_back(value);
  this->has_default_ = 1;
}

template <typename T>
void Action<T>::set_default(const std::vector<T>&& value) {
  default_.clear();
  default_.move(value);
  this->has_default_ = 1;
}

template <typename T>
void Action<T>::set_choices(const std::vector<T>&& value) {
  choices_ = std::move(value);
  this->has_choices_ = 1;
}

template <typename T>
void Action<T>::set_choices(const std::vector<T>& value) {
  choices_ = value;
  this->has_choices_ = 1;
}

template <typename T>
void Action<T>::set_choices(const std::initializer_list<T>& value) {
  choices_ = value;
  this->has_choices_ = 1;
}

template <typename T>
void Action<T>::set_destination(T* destination) {
  this->set_destination(ScalarModel<T>::create(destination));
}

template <typename T>
void Action<T>::set_destination(const std::shared_ptr<StorageModel<T>>& model) {
  destination_ = model;
  has_destination_ = 1;
}

template <class T>
template <class Allocator>
void Action<T>::set_destination(std::list<T, Allocator>* destination) {
  this->set_destination(ListModel<T, Allocator>::create(destination));
}

template <class T>
template <class Allocator>
void Action<T>::set_destination(std::vector<T, Allocator>* destination) {
  this->set_destination(VectorModel<T, Allocator>::create(destination));
}

template <typename T>
bool StoreValue<T>::is_scalar() const {
  return (this->nargs_ == ZERO_OR_ONE || this->nargs_ == EXACTLY_ONE);
}

template <typename T>
std::string StoreValue<T>::get_help(size_t column_width) const {
  std::list<std::string> parts;
  if (this->has_help_) {
    parts.push_back(wrap(this->help_, column_width));
  }
  if (this->has_choices_ && !this->choices_.empty()) {
    parts.push_back(wrap(
        fmt::format("choices=[{}]", stringutil::join(this->choices_, ", ")),
        column_width));
  }
  // if (this->has_default_ && !this->default_.empty()) {
  //   parts.push_back(
  //       wrap(fmt::format("default=[{}]",
  //              stringutil::join(this->default_, ", ")),
  //            column_width));
  // }
  return stringutil::join(parts, "\n");
}

template <typename T>
bool StoreValue<T>::validate() {
  ARGUE_ASSERT(CONFIG_ERROR, !this->has_const_)
      << ".const_= is invalid for action type `store`";
  ARGUE_ASSERT(CONFIG_ERROR, this->has_destination_)
      << ".dest= is required for action type `store`";

  // TODO(josh): should we enable this?
  // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
  // << `store` action must either be required or have a default value set;

  if (this->nargs_ == ZERO_OR_ONE || this->nargs_ == ZERO_OR_MORE) {
    // TODO(josh): should we require that default is specified if argument
    // is optional? Perhaps we can use a sentinel that says default is already
    // stored in the destination?
    // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
    // << `store` action must either be required or have a default value set;
  }

  if (this->has_default_) {
    if (this->is_scalar()) {
      this->destination_->assign(this->default_[0]);
    } else {
      this->destination_->init(this->default_.size());
      for (const auto& elem : this->default_) {
        this->destination_->append(elem);
      }
    }
  }
  return true;
}

template <typename T>
void StoreValue<T>::consume_args(const ParseContext& ctx,
                                 std::list<std::string>* args,
                                 ActionResult* result) {
  if (this->is_scalar()) {
    this->consume_scalar(ctx, args, result);
  } else {
    this->consume_list(ctx, args, result);
  }
}

template <typename T>
void StoreValue<T>::consume_scalar(const ParseContext& ctx,
                                   std::list<std::string>* args,
                                   ActionResult* result) {
  ArgType arg_type = get_arg_type(args->front());
  if (arg_type == POSITIONAL) {
    T value{};
    if (parse(args->front(), &value)) {
      result->code = PARSE_EXCEPTION;
      return;
    }
    if (this->choices_.size() > 0) {
      ARGUE_ASSERT(INPUT_ERROR, has_choice(this->choices_, value))
          << fmt::format("Invalid value '{}' choose from '{}'", args->front(),
                         stringutil::join(this->choices_, ", "));
    }
    this->destination_->assign(value);
    args->pop_front();
  } else {
    ARGUE_THROW(INPUT_ERROR) << fmt::format(
        "Expected a value but instead got a flag {}", ctx.arg.c_str());
  }
}

template <typename T>
void StoreValue<T>::consume_list(const ParseContext& ctx,
                                 std::list<std::string>* args,
                                 ActionResult* result) {
  size_t min_args = 0;
  size_t max_args = 0xffff;
  if (this->nargs_ < 1) {
    switch (this->nargs_) {
      case EXACTLY_ONE:
        min_args = 1;
        max_args = 1;
        break;

      case ZERO_OR_ONE:
        min_args = 0;
        max_args = 1;
        break;

      case ONE_OR_MORE:
        min_args = 1;
        break;

      case ZERO_OR_MORE:
      case REMAINDER:
        break;

      default:
        ARGUE_THROW(CONFIG_ERROR)
            << fmt::format("Invalid nargs {} for list store", this->nargs_);
    }
  } else {
    min_args = this->nargs_;
    max_args = this->nargs_;
  }

  if (max_args < 0xffff) {
    this->destination_->init(max_args);
  } else {
    this->destination_->init(1);
  }

  if (this->nargs_ == REMAINDER && args->front() == "--") {
    args->pop_front();
  }

  T value;
  size_t arg_idx = 0;
  for (arg_idx = 0; arg_idx < max_args && !args->empty(); arg_idx++) {
    ArgType arg_type = get_arg_type(args->front());
    if (arg_type == POSITIONAL || this->nargs_ == REMAINDER) {
      if (parse(args->front(), &value)) {
        result->code = PARSE_EXCEPTION;
        return;
      }

      if (this->choices_.size() > 0) {
        ARGUE_ASSERT(INPUT_ERROR, has_choice(this->choices_, value))
            << fmt::format("Invalid value '{}' choose from '{}'", args->front(),
                           stringutil::join(this->choices_));
      }
      args->pop_front();
      if (this->has_destination_) {
        this->destination_->append(value);
      }

    } else {
      break;
    }
  }

  ARGUE_ASSERT(INPUT_ERROR, arg_idx >= min_args)
      << fmt::format("Expected {} arguments but only got {}", min_args,
                     arg_idx + 1, ctx.arg.c_str());
}

template <typename T>
void StoreConst<T>::set_const(const T& value) {
  const_.clear();
  const_.push_back(value);
  this->has_const_ = 1;
}

template <typename T>
bool StoreConst<T>::validate() {
  ARGUE_ASSERT(CONFIG_ERROR, this->has_const_)
      << "const_= is required for action='store_const'";
  ARGUE_ASSERT(CONFIG_ERROR, this->has_destination_)
      << "dest_= is required for action='store_const'";
  ARGUE_ASSERT(CONFIG_ERROR, !this->has_required_ || !this->required_)
      << "required_ may not be true for action='store_const'";
  // ARGUE_ASSERT(spec.default_.is_set)
  // << "default_= is required for action='store_const'";

  if (this->has_default_) {
    if (this->is_scalar()) {
      this->destination_->assign(this->default_[0]);
    } else {
      this->destination_->init(this->default_.size());
      for (const auto& elem : this->default_) {
        this->destination_->append(elem);
      }
    }
  }
  return true;
}

template <typename T>
void StoreConst<T>::consume_args(const ParseContext& ctx,
                                 std::list<std::string>* args,
                                 ActionResult* result) {
  if (this->has_const_) {
    if (this->is_scalar()) {
      this->destination_->assign(this->const_[0]);
    } else {
      this->destination_->init(this->const_.size());
      for (const auto& elem : this->const_) {
        this->destination_->append(elem);
      }
    }
  }
}

template <typename T>
std::string AppendValue<T>::get_help(size_t column_width) const {
  std::list<std::string> parts;
  if (this->has_help_) {
    parts.push_back(wrap(this->help_, column_width));
  }
  if (this->has_choices_ && !this->choices_.empty()) {
    parts.push_back(wrap(
        fmt::format("choices=[{}]", stringutil::join(this->choices_, ", ")),
        column_width));
  }
  // if (this->has_default_ && !this->default_.empty()) {
  //   parts.push_back(
  //       wrap(fmt::format("default=[{}]",
  //              stringutil::join(this->default_, ", ")),
  //            column_width));
  // }
  return stringutil::join(parts, "\n");
}

template <typename T>
bool AppendValue<T>::validate() {
  ARGUE_ASSERT(CONFIG_ERROR, !this->has_const_)
      << ".const_= is invalid for action type `append`";
  ARGUE_ASSERT(CONFIG_ERROR, this->has_destination_)
      << ".dest= is required for action type `append`";

  // TODO(josh): should we enable this?
  // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
  // << `store` action must either be required or have a default value set;

  if (this->nargs_ == ZERO_OR_ONE || this->nargs_ == ZERO_OR_MORE) {
    // TODO(josh): should we require that default is specified if argument
    // is optional? Perhaps we can use a sentinel that says default is already
    // stored in the destination?
    // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
    // << `store` action must either be required or have a default value set;
  }

  return true;
}

template <typename T>
void AppendValue<T>::consume_args(const ParseContext& ctx,
                                  std::list<std::string>* args,
                                  ActionResult* result) {
  ArgType arg_type = get_arg_type(args->front());
  if (arg_type == POSITIONAL) {
    T value{};
    if (parse(args->front(), &value)) {
      result->code = PARSE_EXCEPTION;
      return;
    }
    if (this->choices_.size() > 0) {
      ARGUE_ASSERT(INPUT_ERROR, has_choice(this->choices_, value))
          << fmt::format("Invalid value '{}' choose from '{}'", args->front(),
                         stringutil::join(this->choices_, ", "));
    }
    this->destination_->append(value);
    args->pop_front();
  } else {
    ARGUE_THROW(INPUT_ERROR) << fmt::format(
        "Expected a value but instead got a flag {}", ctx.arg.c_str());
  }
  result->keep_active = true;
}

}  // namespace argue
