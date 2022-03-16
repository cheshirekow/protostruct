#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/parser.h"

#include "argue/keywords.tcc"

namespace argue {

template <typename T>
void Parser::add_action(const std::string& short_flag,
                        const std::string& long_flag,
                        const std::shared_ptr<Action<T>>& action) {
  ARGUE_ASSERT(CONFIG_ERROR, short_flag.size() > 0 || long_flag.size() > 0)
      << "Cannot add_argument with both short_flag='' and long_flag=''";
  action->set_usage(USAGE_FLAG);

  FlagStore store{
      .short_flag = short_flag, .long_flag = long_flag, .action = action};

  if (long_flag.size() > 0) {
    ARGUE_ASSERT(CONFIG_ERROR, long_flags_.find(long_flag) == long_flags_.end())
        << fmt::format("Duplicate long flag {}", long_flag.c_str());
    long_flags_[long_flag] = store;
  }

  if (short_flag.size() > 0) {
    ARGUE_ASSERT(CONFIG_ERROR,
                 short_flags_.find(short_flag) == short_flags_.end())
        << fmt::format("Duplicate short flag {}", short_flag.c_str());
    short_flags_[short_flag] = store;
  }

  FlagHelp help{
      .short_flag = short_flag, .long_flag = long_flag, .action = action};
  flag_help_.emplace_back(help);
}

template <typename T>
void Parser::add_action(const std::string& name_or_flag,
                        const std::shared_ptr<Action<T>>& action) {
  ARGUE_ASSERT(CONFIG_ERROR, name_or_flag.size() > 0)
      << "Cannot add_argument with empty name_or_flag string";
  ArgType arg_type = get_arg_type(name_or_flag);
  switch (arg_type) {
    case SHORT_FLAG: {
      this->add_action<T>(name_or_flag, std::string(""), action);
      break;
    }

    case LONG_FLAG: {
      this->add_action(std::string(""), name_or_flag, action);
      break;
    }

    case POSITIONAL: {
      action->set_usage(USAGE_POSITIONAL);
      positionals_.emplace_back(action);
      PositionalHelp help{.name = name_or_flag, .action = action};
      positional_help_.emplace_back(help);

      if (action->get_nargs(0) == REMAINDER) {
        ARGUE_ASSERT(CONFIG_ERROR, !remainder_action_)
            << "Only one remainder action is allowed";
        remainder_action_ = action;
      }
      break;
    }

    case SEPARATOR: {
      ARGUE_THROW(CONFIG_ERROR) << "You may not add '--' as a flag";
    }
  }
}

template <typename T>
KWargs<T> Parser::add_argument(const std::string& short_flag,
                               const std::string& long_flag, KWargs<T> spec) {
  this->add_action(short_flag, long_flag, spec.action);
  return spec;
}

template <typename T>
KWargs<typename ElementType<T>::value> Parser::add_argument(
    const std::string& short_flag, const std::string& long_flag, T* dest,
    KWargs<typename ElementType<T>::value> kwargs) {
  if (dest) {
    kwargs.dest = dest;
  }
  return this->add_argument<typename ElementType<T>::value>(short_flag,
                                                            long_flag, kwargs);
}

template <typename T>
KWargs<T> Parser::add_argument(const std::string& name_or_flag,
                               KWargs<T> spec) {
  this->add_action(name_or_flag, spec.action);
  return spec;
}

template <typename T>
KWargs<typename ElementType<T>::value> Parser::add_argument(
    const std::string& name_or_flag, T* dest,
    KWargs<typename ElementType<T>::value> kwargs) {
  if (dest) {
    kwargs.dest = dest;
  }
  return this->add_argument<typename ElementType<T>::value>(name_or_flag,
                                                            kwargs);
}

template <TagNo TAG, class T, class... Args>
void Parser::add_argument(const std::string& short_flag,
                          const std::string& long_flag,
                          const KeywordArgument<TAG, T>& arg0,
                          const Args&... args) {
  auto ctx = MakeHelper<KeywordArgument<TAG, T>, Args...>::make_context();
  HandleSequence(&ctx, arg0, args...);
  this->add_action(short_flag, long_flag, ctx.action);
}

template <TagNo TAG, class T, class... Args>
void Parser::add_argument(const std::string& name_or_flag,
                          const KeywordArgument<TAG, T>& arg0,
                          const Args&... args) {
  auto ctx = MakeHelper<KeywordArgument<TAG, T>, Args...>::make_context();
  HandleSequence(&ctx, arg0, args...);
  this->add_action(name_or_flag, ctx.action);
}

}  // namespace argue
