// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/parser.h"

#include <algorithm>
#include <fstream>

#include "argue/exception.h"
#include "argue/json_dump.h"
#include "argue/parse.h"
#include "tangent/util/fallthrough.h"
#include "tangent/util/nullstream.h"
#include "tangent/util/stdio_filebuf.h"
#include "tangent/util/stringutil.h"

#include "argue/action.tcc"
#include "argue/kwargs.tcc"
#include "argue/parser.tcc"

extern "C" {
const char* ARGUE_AUTOCOMPLETE_ME = "YES";
};

namespace argue {

// =============================================================================
//                             Parser Utils
// =============================================================================

std::string get_flag_usage(const std::string& short_flag,
                           const std::string& long_flag,
                           const std::shared_ptr<ActionBase>& action) {
  std::stringstream token;
  if (!action->is_required()) {
    token << "[";
  }

  std::list<std::string> parts;
  std::list<std::string> names;
  if (short_flag.size()) {
    names.emplace_back(short_flag);
  }

  std::string default_metavar = "??";
  if (!long_flag.empty()) {
    names.emplace_back(long_flag);
    default_metavar = long_flag.substr(2);
  }

  std::string name = stringutil::join(names, "/");

  int nargs = action->get_nargs(EXACTLY_ONE);
  std::string metavar =
      action->get_metavar(stringutil::to_upper(default_metavar));
  if (nargs == ONE_OR_MORE) {
    parts = {name, metavar + " [..]"};
  } else if (nargs == ZERO_OR_ONE) {
    parts = {name, "[" + metavar + "]"};
  } else if (nargs == ZERO_OR_MORE) {
    parts = {name, "[" + metavar + " [..]]"};
  } else if (nargs == EXACTLY_ONE) {
    parts = {name};
  } else if (nargs > 0) {
    parts = {name, metavar, metavar, ".."};
  }

  token << stringutil::join(parts, " ");
  if (!action->is_required()) {
    token << "]";
  }

  return token.str();
}

std::string get_positional_usage(const std::string& name,
                                 const std::shared_ptr<ActionBase>& action) {
  int nargs = action->get_nargs(EXACTLY_ONE);
  std::string metavar = action->get_metavar(stringutil::to_upper(name));

  if (nargs == ONE_OR_MORE) {
    return "<" + metavar + "> [" + metavar + "..]";
  } else if (nargs == ZERO_OR_ONE) {
    return "[" + metavar + "]";
  } else if (nargs == ZERO_OR_MORE) {
    return "[" + metavar + " [" + metavar + "..]]";
  } else if (nargs == EXACTLY_ONE) {
    return "<" + metavar + ">";
  } else if (nargs > 0) {
    return fmt::format("<{0}> [{0}..]({1})", metavar, nargs);
  }

  return "";
}

const ColumnSpec kDefaultColumns = {4, 16, 60};

std::string repeat(const std::string bit, int n) {
  std::stringstream out;
  for (int i = 0; i < n; i++) {
    out << bit;
  }
  return out.str();
}

// http://rosettacode.org/wiki/Word_wrap#C.2B.2B
std::string wrap(const std::string text, size_t line_length) {
  if (line_length == 0) {
    return text;
  }
  std::istringstream words(text);
  std::ostringstream wrapped;
  std::string word;

  if (words >> word) {
    wrapped << word;
    size_t space_left = line_length - word.length();
    while (words >> word) {
      if (space_left < word.length() + 1) {
        wrapped << '\n' << word;
        space_left = line_length - word.length();
      } else {
        wrapped << ' ' << word;
        space_left -= word.length() + 1;
      }
    }
  }
  return wrapped.str();
}

VersionString::VersionString(int major, int minor, int patch) {
  std::stringstream strstrm{};
  strstrm << major << "." << minor << "." << patch;
  this->assign(strstrm.str());
}

VersionString::VersionString(int major, int minor, int patch, const char* tag,
                             int increment) {
  std::stringstream strstrm{};
  strstrm << major << "." << minor << "." << patch << "-" << tag << increment;
  this->assign(strstrm.str());
}

// =============================================================================
//                                 Parser
// =============================================================================

Parser::Parser(const Metadata& meta) : meta_(meta) {
  if (meta.add_help) {
    this->add_argument<void>("-h", "--help", {.action = "help"});
  }
  if (meta.add_version) {
    this->add_argument<void>("--version", KWargs<void>{.action = "version"});
  }
}

std::shared_ptr<Subparsers> Parser::add_subparsers(
    const std::string& name, std::string* dest, const SubparserOptions& opts) {
  Subparsers::Metadata submeta{};
  submeta.command_prefix = meta_.command_prefix + " " + meta_.name;
  submeta.subdepth = meta_.subdepth + 1;
  std::shared_ptr<Subparsers> action = std::make_shared<Subparsers>(submeta);

  KWargs<std::string> spec{};
  spec.action = action;
  spec.nargs = EXACTLY_ONE;
  spec.required = true;
  spec.help = opts.help;
  spec.dest = dest;
  spec.metavar = name;

  positionals_.emplace_back(action);
  PositionalHelp help{.name = name, .action = action};

  positional_help_.emplace_back(help);
  subcommand_help_.emplace_back(action);
  return action;
}

int Parser::parse_args(int argc, char** argv, std::ostream* out) {
  if (argc < 0) {
    return PARSE_EXCEPTION;
  }
  if (argc > 0) {
    if (meta_.name.empty()) {
      meta_.name = argv[0];
    }
  }

  std::list<std::string> args;
  for (size_t i = 1; i < static_cast<size_t>(argc); ++i) {
    args.emplace_back(argv[i]);
  }

  return parse_args(&args, out);
}

int Parser::parse_args(const std::initializer_list<std::string>& init_list,
                       std::ostream* out) {
  std::list<std::string> args = init_list;
  return parse_args(&args, out);
}

static AutoCompleteContext maybe_autocomplete(std::list<std::string>* args) {
  AutoCompleteContext ctx{};

  const char* value = nullptr;

  value = getenv("_ARGUE_DEBUG");
  // TODO(josh): intentional memory leak :(.
  if (value && std::stoi(value) == 1) {
    ctx.debug = new std::ofstream{"/tmp/argue-complete.log"};
  } else {
    ctx.debug = new NullStream{};
  }

  // NOTE(josh): this is really just here to ensure that the sentinel symbol
  // can't be stripped
  value = getenv("_ARGUE_HAS_CAN_COMPLETE");
  if (value && std::strcmp(value, "1") == 0) {
    (*ctx.debug) << "HAS_ARGUE_AUTOCOMPLETE_ME: " << ARGUE_AUTOCOMPLETE_ME
                 << "\n";
  }

  value = getenv("_ARGUECOMPLETE");
  if (!value) {
    return ctx;
  }
  if (std::strcmp(value, "1") != 0) {
    return ctx;
  }

  // NOTE(josh): __gnu_cxx::stdio_filebuf documentation says it will close the
  // file descriptor on destruction.
  // __gnu_cxx::stdio_filebuf<char> gnu_filebuf(dup(9), std::ios::out);
  // std::ostream out{&gnu_filebuf};

  std::vector<std::string> envs = {
      "_ARGUECOMPLETE", "_ARGUECOMPLETE_IFS", "COMP_LINE",
      "COMP_POINT",     "COMP_TYPE",          "COMP_CWORD",
      "COMP_KEY",       "COMP_WORDBREAKS",    "COMP_WORDS"};

  for (const std::string& env : envs) {
    value = getenv(env.c_str());
    if (value) {
      (*ctx.debug) << env << ": \"" << value << "\"\n";
    }
  }

  (*ctx.debug) << "args: [\n";
  for (const std::string& arg : *args) {
    (*ctx.debug) << "  " << arg << "\n";
  }
  (*ctx.debug) << "]\n";
  ctx.debug->flush();

  value = getenv("_ARGUECOMPLETE_IFS");
  if (value) {
    ctx.ifs = value;
  } else {
    ctx.ifs = static_cast<char>(013);
  }

  value = getenv("COMP_CWORD");
  if (!value) {
    (*ctx.debug) << "no CWORD" << std::endl;
    return ctx;
  }

  size_t comp_word_idx = std::stoul(value);
  if (args->size() < comp_word_idx) {
    (*ctx.debug) << "CWORD > argn" << std::endl;
    return ctx;
  }

  auto iter = args->begin();
  for (size_t idx = 0; idx < comp_word_idx - 1; idx++) {
    iter++;
  }
  (*ctx.debug) << "Stored pointer to comp_word: " << (*iter) << "\n";
  ctx.debug->flush();

  ctx.active = true;
  ctx.comp_word = iter;
  return ctx;
}

int Parser::parse_args(std::list<std::string>* args, std::ostream* out) {
  try {
    ParseContext ctx{};
    ctx.out = out;
    ctx.auto_complete = maybe_autocomplete(args);
    return parse_args_impl(args, ctx);
  } catch (const Exception& ex) {
    (*out) << Exception::to_string(ex.typeno) << ": ";
    (*out) << ex.message << "\n";
    if (ex.typeno == Exception::BUG) {
      (*out) << ex.stack_trace;
    }
    return PARSE_EXCEPTION;
  }
}

void Parser::validate() {
  for (auto& action : positionals_) {
    action->validate();
  }
  for (auto& pair : short_flags_) {
    pair.second.action->validate();
  }
  for (auto& pair : long_flags_) {
    pair.second.action->validate();
  }
}

int Parser::autocomplete(const ParseContext& ctx) {
  std::string comp_word = ctx.arg;
  (*ctx.auto_complete.debug) << "Completing word: " << comp_word << "\n";
  ctx.auto_complete.debug->flush();

  if (comp_word.size() > 1 && comp_word[0] == '-' && comp_word[1] != '-') {
    (*ctx.auto_complete.debug)
        << "Completion is a shortflag (or short flag group)\n";
    // The completion word is a short flag, or a short-flag collection.
    // Regardless the completion is the list of available short flags that
    // aren't already in the collection.
    (*ctx.auto_complete.debug) << "Available short flags: ";
    for (auto& flag_pair : short_flags_m_) {
      FlagStore store = flag_pair.second;
      (*ctx.auto_complete.debug) << store.short_flag[1] << ", ";
      std::cout << store.short_flag[1] << ctx.auto_complete.ifs;
    }
    (*ctx.auto_complete.debug) << std::endl;
    std::cout.flush();
    exit(0);
    return 0;
  }

  (*ctx.auto_complete.debug) << "Completion is anything\n";

  for (auto& flag_pair : short_flags_m_) {
    FlagStore store = flag_pair.second;
    if (stringutil::starts_with(store.short_flag, comp_word)) {
      (*ctx.auto_complete.debug) << store.short_flag << "\n";
      std::cout << store.short_flag << ctx.auto_complete.ifs;
    } else {
      (*ctx.auto_complete.debug)
          << store.short_flag << " doesn't start with " << comp_word << "\n";
    }
  }

  for (auto& flag_pair : long_flags_m_) {
    FlagStore store = flag_pair.second;
    if (stringutil::starts_with(store.long_flag, comp_word)) {
      (*ctx.auto_complete.debug) << store.long_flag << "\n";
      std::cout << store.long_flag << ctx.auto_complete.ifs;
    } else {
      (*ctx.auto_complete.debug)
          << store.long_flag << " doesn't start with " << comp_word << "\n";
    }
  }

  for (auto& positional : positionals_m_) {
    positional->write_completions(ctx);
  }

  (*ctx.auto_complete.debug).flush();
  std::cout.flush();
  exit(0);
  return 0;
}

int Parser::parse_args_impl(std::list<std::string>* args,
                            const ParseContext& parent_ctx) {
  this->validate();
  ParseContext ctx{parent_ctx};
  ctx.parser = this;

  // Create mutable copies of each argument set so we can remove actions
  // when they are encountered
  positionals_m_ = positionals_;
  short_flags_m_ = short_flags_;
  long_flags_m_ = long_flags_;

  while (args->size() > 0) {
    if (ctx.auto_complete.active &&
        ctx.auto_complete.comp_word == args->begin()) {
      ctx.arg = args->front();
      args->pop_front();
      return autocomplete(ctx);
    }

    ArgType arg_type = get_arg_type(args->front());
    ActionResult out{
        .keep_active = false,
        .code = PARSE_FINISHED,
    };

    switch (arg_type) {
      case SHORT_FLAG: {
        ctx.arg = args->front();
        args->pop_front();
        for (size_t idx = 1; idx < ctx.arg.size(); ++idx) {
          std::string query_flag = std::string("-") + ctx.arg[idx];
          auto flag_iter = short_flags_m_.find(query_flag);
          ARGUE_ASSERT(INPUT_ERROR, (flag_iter != short_flags_m_.end()))
              << "Unrecognized short flag: " << query_flag;
          FlagStore store = flag_iter->second;
          ARGUE_ASSERT(BUG, static_cast<bool>(store.action))
              << "Flag " << query_flag
              << " was found in index with empty action pointer";
          store.action->consume_args(ctx, args, &out);

          if (!out.keep_active) {
            short_flags_m_.erase(store.short_flag);
            long_flags_m_.erase(store.long_flag);
          }
        }
        break;
      }

      case LONG_FLAG: {
        ctx.arg = args->front();
        args->pop_front();
        auto flag_iter = long_flags_m_.find(ctx.arg);
        size_t prefix_matches = 0;
        if (flag_iter == long_flags_m_.end()) {
          // We didn't find an exact match for this flag, so let's look for a
          // a unique prefix match. If one exists, we'll use that.
          for (auto search_iter = long_flags_m_.begin();
               search_iter != long_flags_m_.end(); search_iter++) {
            if (stringutil::starts_with(search_iter->first, ctx.arg)) {
              flag_iter = search_iter;
              prefix_matches++;
            }
          }
        }

        ARGUE_ASSERT(INPUT_ERROR, flag_iter != long_flags_m_.end())
            << "Unrecognized long flag: " << ctx.arg;
        ARGUE_ASSERT(INPUT_ERROR, prefix_matches < 2)
            << "Long flag '" << ctx.arg
            << "' is not unique as an implicit prefix of known flags";

        FlagStore store = flag_iter->second;
        ARGUE_ASSERT(BUG, static_cast<bool>(store.action))
            << "Flag " << ctx.arg
            << " was found in index with empty action pointer";
        store.action->consume_args(ctx, args, &out);
        if (!out.keep_active) {
          short_flags_m_.erase(store.short_flag);
          long_flags_m_.erase(store.long_flag);
        }
        break;
      }

      case POSITIONAL:
        TANGENT_FALLTHROUGH
      case SEPARATOR: {
        ctx.arg = "";
        ARGUE_ASSERT(CONFIG_ERROR, positionals_m_.size() > 0)
            << "Additional positional arguments with no available actions "
               "remaining: '"
            << args->front() << "'";
        std::shared_ptr<ActionBase> action = positionals_m_.front();
        positionals_m_.pop_front();
        ARGUE_ASSERT(BUG, static_cast<bool>(action))
            << "positional with empty action pointer";
        action->consume_args(ctx, args, &out);
        break;
      }
    }

    if (out.code != PARSE_FINISHED) {
      return out.code;
    }
  }

  for (const std::shared_ptr<ActionBase>& action : positionals_m_) {
    ARGUE_ASSERT(INPUT_ERROR, !action->is_required())
        << "Missing required positional\n"
        << get_usage_string();
  }

  for (const auto& pair : short_flags_) {
    const FlagStore& store = pair.second;
    ARGUE_ASSERT(INPUT_ERROR, !store.action->is_required())
        << "Missing required flag (" << store.short_flag << ","
        << store.long_flag << ")" << get_usage_string();
  }

  for (const auto& pair : long_flags_) {
    const FlagStore& store = pair.second;
    ARGUE_ASSERT(INPUT_ERROR, !store.action->is_required())
        << "Missing required flag (" << store.short_flag << ","
        << store.long_flag << ")" << get_usage_string();
  }

  return PARSE_FINISHED;
}

void Parser::print_usage(std::ostream* out, size_t width) {
  std::stringstream line;

  std::list<std::string> parts;
  if (!meta_.command_prefix.empty()) {
    parts.push_back(meta_.command_prefix);
  }
  parts.push_back(meta_.name);
  for (const FlagHelp& help : flag_help_) {
    parts.push_back(
        get_flag_usage(help.short_flag, help.long_flag, help.action));
  }

  for (const PositionalHelp& help : positional_help_) {
    parts.push_back(get_positional_usage(help.name, help.action));
  }

  (*out) << stringutil::join(parts, " ") << "\n";
}

std::string Parser::get_usage_string() {
  std::stringstream strm{};
  print_usage(&strm);
  return strm.str();
}

static void print_columns(std::ostream* out, const ColumnSpec& columns,
                          const std::string& name,
                          const std::string& description) {
  size_t width = 80;
  size_t padding = (width - container_sum(columns)) / (columns.size() - 1);

  if (name.size() > padding + columns[0] + columns[1]) {
    (*out) << "\n";
  }
  (*out) << name;
  (*out) << repeat(" ", 2 * padding + columns[0] + columns[1] - name.size());
  if (name.size() > padding + columns[0] + columns[1]) {
    (*out) << "\n";
    (*out) << repeat(" ", columns[0] + columns[1] + 2 * padding);
  }

  std::stringstream ss(description);
  std::string line;

  if (std::getline(ss, line, '\n')) {
    (*out) << line << "\n";
  } else {
    (*out) << "\n";
  }

  while (std::getline(ss, line, '\n')) {
    (*out) << repeat(" ", columns[0] + columns[1] + 2 * padding) << line
           << "\n";
  }
}

void Parser::print_help(std::ostream* out, const HelpOptions& opts) {
  if (opts.format == HelpOptions::FORMAT_JSON) {
    print_helpJSON(out, opts);
  } else {
    print_helpText(out, opts);
  }

  if (opts.recurse) {
    for (const auto& sub : subcommand_help_) {
      for (auto& pair : *sub) {
        // TODO(josh): writing the comma here is really hacky. Seems like it
        // would be better to pass the dumper down the stack instead of the
        // output stream. But then we need the JSON headers in the argue
        // headers and I don't think we want that.
        (*out) << ",";
        pair.second->print_help(out, opts);
      }
    }
  }
}

void Parser::print_helpJSON(std::ostream* out, const HelpOptions& opts) {
  // TODO(josh): Use a registry??
  StreamDumper dumper{out};
  DumpGuard object{&dumper, GUARD_OBJECT};
  dumper.dump_field_prefix("metadata");
  {
    DumpGuard object{&dumper, GUARD_OBJECT};
    dumper.dump_field("id", fmt::format("{:p}", static_cast<void*>(this)));
    dumper.dump_field("name", meta_.name);
    dumper.dump_field("author", meta_.author);
    dumper.dump_field("copyright", meta_.copyright);
    dumper.dump_field("prolog", meta_.prolog);
    dumper.dump_field("epilog", meta_.epilog);
    dumper.dump_field("comamnd_prefix", meta_.command_prefix);
    dumper.dump_field("subdepth", meta_.subdepth);
    dumper.dump_field("usage", get_usage_string());
  }

  dumper.dump_field_prefix("flags");
  {
    DumpGuard object{&dumper, GUARD_LIST};
    for (const FlagHelp& help : flag_help_) {
      dumper.dump_event(DumpEvent::LIST_VALUE);
      DumpGuard object{&dumper, GUARD_OBJECT};
      dumper.dump_field("short_flag", help.short_flag);
      dumper.dump_field("long_flag", help.long_flag);
      dumper.dump_field("help", help.action->get_help());
    }
  }

  dumper.dump_field_prefix("positional");
  {
    DumpGuard object{&dumper, GUARD_LIST};
    for (const PositionalHelp& help : positional_help_) {
      dumper.dump_event(DumpEvent::LIST_VALUE);
      DumpGuard object{&dumper, GUARD_OBJECT};
      dumper.dump_field("name", help.name);
      dumper.dump_field("help", help.action->get_help());
    }
  }

  dumper.dump_field_prefix("subcommands");
  {
    DumpGuard object{&dumper, GUARD_LIST};
    for (const auto& sub : subcommand_help_) {
      for (auto& pair : *sub) {
        dumper.dump_event(DumpEvent::LIST_VALUE);
        DumpGuard object{&dumper, GUARD_OBJECT};
        dumper.dump_field(
            "id", fmt::format("{:p}", static_cast<void*>(pair.second.get())));
        dumper.dump_field("name", pair.first);
        dumper.dump_field("help", pair.second->get_prolog());
      }
    }
  }
}

void Parser::print_helpText(std::ostream* out, const HelpOptions& opts) {
  const ColumnSpec columns = opts.columns;
  size_t width = 80;
  size_t padding = (width - container_sum(columns)) / (columns.size() - 1);

  // TODO(josh): detect multiline and break it up
  if (meta_.subdepth < 1) {
    (*out) << repeat("=", meta_.name.size()) << "\n"
           << meta_.name << "\n"
           << repeat("=", meta_.name.size()) << "\n";

    if (!meta_.version.empty()) {
      (*out) << "version: " << meta_.version << "\n";
    }
    if (!meta_.author.empty() > 0) {
      (*out) << "author : " << meta_.author << "\n";
    }
    if (!meta_.copyright.empty() > 0) {
      (*out) << "copyright: " << meta_.copyright << "\n";
    }
    (*out) << "\n";
  }

  print_usage(out, width);

  if (meta_.prolog.size() > 0) {
    (*out) << "\n" << meta_.prolog << "\n";
  }

  if (flag_help_.size() > 0) {
    if (opts.depth < 1) {
      (*out) << "\n";
      (*out) << "Flags:\n";
      (*out) << repeat("-", 6) << "\n";
    } else {
      (*out) << repeat("-", 4) << "\n";
    }
    for (const FlagHelp& help : flag_help_) {
      // If we're going to overflow our column, then also push a newline
      // between us and the previous one for some additional padding
      if (help.long_flag.size() > columns[1]) {
        (*out) << "\n";
      }
      (*out) << help.short_flag;
      (*out) << repeat(" ", padding + columns[0] - help.short_flag.size());
      (*out) << help.long_flag;
      (*out) << repeat(" ", padding + columns[1] - help.long_flag.size());

      // If we overflowed the column, then add a new line so that we can
      // start the help text at the right column
      if (help.long_flag.size() > columns[1]) {
        (*out) << "\n";
        (*out) << repeat(" ", columns[0] + columns[1] + 2 * padding);
      }

      std::stringstream ss(help.action->get_help(columns[2]));
      std::string line;

      if (std::getline(ss, line, '\n')) {
        (*out) << line << "\n";
      } else {
        (*out) << "\n";
      }

      while (std::getline(ss, line, '\n')) {
        (*out) << repeat(" ", columns[0] + columns[1] + 2 * padding) << line
               << "\n";
      }
    }
  }

  if (positional_help_.size() > 0) {
    if (opts.depth < 1) {
      (*out) << "\n";
      (*out) << "Positionals:\n";
      (*out) << repeat("-", 12) << "\n";
    } else {
      (*out) << repeat("-", 4) << "\n";
    }
    for (const PositionalHelp& help : positional_help_) {
      print_columns(out, columns, help.name, help.action->get_help(columns[2]));
    }
  }

  if (opts.depth < 1 && subcommand_help_.size() > 0) {
    (*out) << "\n";
    (*out) << "Subcommands:\n" << repeat("-", 12) << "\n";
    for (const auto& sub : subcommand_help_) {
      for (auto& pair : *sub) {
        print_columns(out, columns, pair.first,
                      pair.second->get_prolog(columns[2]));
      }
    }
  }

  if (meta_.epilog.size() > 0) {
    (*out) << meta_.epilog;
  }
}

void Parser::print_version(std::ostream* out, const ColumnSpec& columns) {
  // TODO(josh): detect multiline and break it up
  (*out) << meta_.name << " ";
  if (meta_.version.size() > 0) {
    (*out) << "  version " << meta_.version << "\n";
  }
}

std::string Parser::get_prolog(size_t column_width) {
  return wrap(meta_.prolog, column_width);
}

}  // namespace argue
