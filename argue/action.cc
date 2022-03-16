// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/action.h"

#include <algorithm>
#include <fstream>

#include "argue/exception.h"
#include "argue/parse.h"
#include "argue/parser.h"
#include "tangent/util/stringutil.h"

#include "argue/action.tcc"

namespace argue {

// =============================================================================
//                              Actions
// =============================================================================

ActionBase::ActionBase()
    : usage_(USAGE_POSITIONAL),
      has_nargs_{0},
      has_const_{0},
      has_default_{0},
      has_choices_{0},
      has_required_{0},
      has_help_{0},
      has_metavar_{0},
      has_destination_{0},
      nargs_(EXACTLY_ONE),
      required_{false},
      parser_{nullptr} {}

ActionBase::~ActionBase() {}

// KWargs interface
void ActionBase::set_nargs(int nargs) {
  nargs_ = nargs;
  has_nargs_ = 1;
}
void ActionBase::set_required(bool required) {
  required_ = required;
  has_required_ = 1;
}
void ActionBase::set_help(const std::string& help) {
  help_ = help;
  has_help_ = 1;
}
void ActionBase::set_metavar(const std::string& metavar) {
  metavar_ = metavar;
  has_metavar_ = 1;
}
void ActionBase::set_usage(Usage usage) {
  usage_ = usage;
}

bool ActionBase::validate() {
  return true;
}

bool ActionBase::is_required() const {
  if (usage_ == USAGE_POSITIONAL) {
    if (!has_nargs_) {
      return true;
    }

    if (nargs_ == ZERO_OR_MORE || nargs_ == ZERO_OR_ONE ||
        nargs_ == REMAINDER) {
      return false;
    }

    return true;
  } else {
    return (has_required_ && required_);
  }
}

int ActionBase::get_nargs(int default_value) const {
  if (has_nargs_) {
    return nargs_;
  } else {
    return default_value;
  }
}

std::string ActionBase::get_metavar(const std::string& default_value) const {
  if (has_metavar_) {
    return metavar_;
  } else {
    return default_value;
  }
}

std::string ActionBase::get_help(size_t column_width) const {
  if (this->has_help_) {
    return wrap(this->help_, column_width);
  } else {
    return "";
  }
}

void ActionBase::set_parser(Parser* parser) {
  ARGUE_ASSERT(CONFIG_ERROR, !parser_)
      << "Invalid re-use of action object for " << type_name_;
  parser_ = parser;
}

Subparsers::Subparsers(const Metadata& metadata) : metadata_(metadata) {}

bool Subparsers::validate() {
  bool ok = StoreValue<std::string>::validate();
  for (auto& pair : subparser_map_) {
    pair.second->validate();
  }
  return ok;
}

std::string Subparsers::get_help(size_t column_width) const {
  std::list<std::string> parts;
  if (!this->subparser_map_.empty()) {
    parts.push_back(fmt::format(
        "[{}]", stringutil::join(keys(this->subparser_map_), ", ")));
  }
  if (this->has_help_) {
    parts.push_back(this->help_);
  }
  return stringutil::join(parts, "\n");
}

void Subparsers::consume_args(const ParseContext& ctx,
                              std::list<std::string>* args,
                              ActionResult* result) {
  ARGUE_ASSERT(CONFIG_ERROR, this->nargs_ == EXACTLY_ONE)
      << fmt::format("Invalid nargs_={}", this->nargs_);
  ArgType arg_type = get_arg_type(args->front());

  if (arg_type == POSITIONAL) {
    std::string local_command;

    int parse_result = parse(args->front(), &local_command);
    ARGUE_ASSERT(INPUT_ERROR, parse_result == 0)
        << fmt::format("Unable to parse command '{}'", args->front());
    args->pop_front();

    auto iter = subparser_map_.find(local_command);
    ARGUE_ASSERT(INPUT_ERROR, iter != subparser_map_.end())
        << "Invalid value '" << local_command << "' choose from '"
        << stringutil::join(keys(subparser_map_), "', '") << "'";

    if (this->destination_) {
      this->destination_->assign(local_command);
    }
    std::shared_ptr<Parser> subparser = iter->second;
    result->code =
        static_cast<ParseResult>(subparser->parse_args_impl(args, ctx));
  } else {
    ARGUE_ASSERT(INPUT_ERROR, false) << fmt::format(
        "Expected a command name but instead got a flag {}", ctx.arg.c_str());
  }
}

Subparsers::MapType::const_iterator Subparsers::begin() const {
  return subparser_map_.begin();
}

Subparsers::MapType::const_iterator Subparsers::end() const {
  return subparser_map_.end();
}

std::shared_ptr<Parser> Subparsers::add_parser(const std::string& command,
                                               const SubparserOptions& opts) {
  auto iter = subparser_map_.find(command);
  if (iter == subparser_map_.end()) {
    Parser::Metadata meta{};
    meta.add_help = true;
    meta.add_version = false;
    meta.name = command;
    meta.prolog = opts.help;
    meta.command_prefix = metadata_.command_prefix;
    meta.subdepth = metadata_.subdepth;
    std::shared_ptr<Parser> parser{new Parser{meta}};
    std::tie(iter, std::ignore) =
        subparser_map_.emplace(std::make_pair(command, parser));
  }

  return iter->second;
}

void Subparsers::write_completions(const ParseContext& ctx) {
  for (auto pair : subparser_map_) {
    if (stringutil::starts_with(pair.first, ctx.arg)) {
      (*ctx.auto_complete.debug) << pair.first << "\n";
      std::cout << pair.first << ctx.auto_complete.ifs;
    } else {
      (*ctx.auto_complete.debug) << pair.first << " doesn't match \n";
    }
  }
}

std::string Help::get_help(size_t column_width) const {
  return wrap("print this help message", column_width);
}

bool Help::validate() {
  return true;
}

void Help::consume_args(const ParseContext& ctx, std::list<std::string>* args,
                        ActionResult* result) {
  Parser::HelpOptions opts{kDefaultColumns, 0};
  char* envstr = getenv("ARGUE_HELP_FORMAT");
  if (envstr && std::string(envstr) == "json") {
    opts.format = Parser::HelpOptions::FORMAT_JSON;
  }

  envstr = getenv("ARGUE_HELP_RECURSE");
  if (envstr && std::string(envstr) == "1") {
    opts.recurse = 1;
  }

  // TODO(josh): dumping the outer list brackets here is kindof hacky... seems
  // like we would really like to pass the json::stream::Dumper instance down
  // the stack. Then we can open the list here using the dump API.
  if (opts.format == Parser::HelpOptions::FORMAT_JSON) {
    (*ctx.out) << "[\n";
  }

  ctx.parser->print_help(ctx.out, opts);
  if (opts.format == Parser::HelpOptions::FORMAT_JSON) {
    (*ctx.out) << "]\n";
  }
  result->code = PARSE_ABORTED;
}

std::string Version::get_help(size_t column_width) const {
  return wrap("print version information and exit", column_width);
}

bool Version::validate() {
  return true;
}

void Version::consume_args(const ParseContext& ctx,
                           std::list<std::string>* args, ActionResult* result) {
  ctx.parser->print_version(ctx.out);
  result->code = PARSE_ABORTED;
}

}  // namespace argue
