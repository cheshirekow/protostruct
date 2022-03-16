#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/keywords.h"

namespace argue {

template <class T, class U>
void AssignmentHelper<TAG_ACTION>::assign(KeywordContext<T>* ctx,
                                          const std::shared_ptr<U>& argument) {
  ctx->action = argument;
}

template <class T>
void AssignmentHelper<TAG_ACTION>::assign(KeywordContext<T>* ctx,
                                          const char* named_action) {
  std::shared_ptr<Action<T>> action;
  if (strcmp(named_action, "store") == 0) {
    ctx->action = std::make_shared<StoreValue<T>>();
  } else if (strcmp(named_action, "store_const") == 0) {
    ctx->action = std::make_shared<StoreConst<T>>();
  } else if (strcmp(named_action, "append") == 0) {
    ctx->action = std::make_shared<AppendValue<T>>();
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false) << fmt::format(
        "invalid action={} for type={}", named_action, type_string<T>());
  }
}

template <>
inline void AssignmentHelper<TAG_ACTION>::assign(KeywordContext<bool>* ctx,
                                                 const char* named_action) {
  std::shared_ptr<Action<bool>> action;
  if (strcmp(named_action, "store") == 0) {
    ctx->action = std::make_shared<StoreValue<bool>>();
  } else if (strcmp(named_action, "store_const") == 0) {
    ctx->action = std::make_shared<StoreConst<bool>>();
  } else if (strcmp(named_action, "store_true") == 0) {
    ctx->action = std::make_shared<StoreConst<bool>>();
    ctx->action->set_default(false);
    ctx->action->set_const(true);
  } else if (strcmp(named_action, "store_false") == 0) {
    ctx->action = std::make_shared<StoreConst<bool>>();
    ctx->action->set_default(false);
    ctx->action->set_const(true);
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false)
        << fmt::format("invalid action={} for type=bool", named_action);
  }
}

template <class T>
void AssignmentHelper<TAG_NARGS>::assign(KeywordContext<T>* ctx, int value) {
  ctx->action->set_nargs(value);
}

template <class T>
void AssignmentHelper<TAG_NARGS>::assign(KeywordContext<T>* ctx,
                                         const char* str) {
  int value = string_to_nargs(str);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", str);
  assign(ctx, value);
}

template <class T>
void AssignmentHelper<TAG_NARGS>::assign(KeywordContext<T>* ctx, const char c) {
  int value = string_to_nargs(c);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", c);
  assign(ctx, value);
}

template <class T>
void AssignmentHelper<TAG_CONST>::assign(KeywordContext<T>* ctx,
                                         const T& value) {
  ctx->action->set_const(value);
}

template <class T>
void AssignmentHelper<TAG_DEFAULT>::assign(KeywordContext<T>* ctx,
                                           const T& value) {
  ctx->action->set_default(value);
}

template <class T>
void AssignmentHelper<TAG_DEFAULT>::assign(KeywordContext<T>* ctx,
                                           const char* value) {
  ctx->action->set_default(value);
}

template <class T>
void AssignmentHelper<TAG_CHOICES>::assign(
    KeywordContext<T>* ctx, const std::initializer_list<T>& choices) {
  std::vector<T> temp = choices;
  ctx->action->set_choices(choices);
}

template <class T>
void AssignmentHelper<TAG_CHOICES>::assign(KeywordContext<T>* ctx,
                                           const std::vector<T>& choices) {
  ctx->action->set_choices(choices);
}

inline void AssignmentHelper<TAG_CHOICES>::assign(
    KeywordContext<std::string>* ctx, const std::vector<const char*>& choices) {
  std::vector<std::string> string_choices(choices.begin(), choices.end());
  ctx->action->set_choices(string_choices);
}

template <class T>
void AssignmentHelper<TAG_DEST>::assign(KeywordContext<T>* ctx,
                                        T* destination) {
  ctx->action->set_destination(destination);
}

template <class T, class Allocator>
void AssignmentHelper<TAG_DEST>::assign(
    KeywordContext<T>* ctx, std::vector<T, Allocator>* destination) {
  std::shared_ptr<StorageModel<T>> model =
      std::make_shared<VectorModel<T, Allocator>>(destination);
  ctx->action->set_destination(model);
}

template <class T, class Allocator>
void AssignmentHelper<TAG_DEST>::assign(KeywordContext<T>* ctx,
                                        std::list<T, Allocator>* destination) {
  std::shared_ptr<StorageModel<T>> model =
      std::make_shared<ListModel<T, Allocator>>(destination);
  ctx->action->set_destination(model);
}

template <class T>
void AssignmentHelper<TAG_REQUIRED>::assign(KeywordContext<T>* ctx,
                                            bool value) {
  ctx->action->set_required(value);
}

template <class T>
void AssignmentHelper<TAG_HELP>::assign(KeywordContext<T>* ctx,
                                        const std::string& value) {
  ctx->action->set_help(value);
}

template <class T>
void AssignmentHelper<TAG_METAVAR>::assign(KeywordContext<T>* ctx,
                                           const char* value) {
  ctx->action->set_metavar(value);
}

}  // namespace argue
