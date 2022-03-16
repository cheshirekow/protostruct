#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "argue/action.h"
#include "argue/keywords.h"
#include "argue/kwargs.h"
#include "argue/util.h"

namespace argue {

// =============================================================================
//                             Parser Utils
// =============================================================================

// Number of character columns width for each of the three columns of help
// text: 1. short flag, 2. long flag, 3. description
typedef std::array<size_t, 3> ColumnSpec;
extern const ColumnSpec kDefaultColumns;

// Help entry for a flag argument
struct FlagHelp {
  std::string short_flag;              //< two character version, like '-h'
  std::string long_flag;               //< dash-dash version, like '--help'
  std::shared_ptr<ActionBase> action;  //< action associated with these flags
};

// Help entry for a positional argument
struct PositionalHelp {
  std::string name;  //< argument name, used as default metavar and as
                     //  an indicator in any error messages
  std::shared_ptr<ActionBase> action;  //< action associated with this argument
};

// Construct a usage string for a flag argument.
/* Generates a string like "--foo <FOO>" */
std::string get_flag_usage(const std::string& short_flag,
                           const std::string& long_flag,
                           const std::shared_ptr<ActionBase>& action);

// Construct a usage string for a positional argument.
/* Generates a string like "<FOO>" */
std::string get_positional_usage(const std::string& name,
                                 const std::shared_ptr<ActionBase>& options);

// Value type for flag maps, allows us to reverse loop up in each list.
struct FlagStore {
  std::string short_flag;  //< The short flag for this action, if it exists
  std::string long_flag;   //< The long flag for this action, if it exists
  std::shared_ptr<ActionBase> action;  //< the action associated with the flag
};

// Helper to convert version tuple to a string
class VersionString : public std::string {
 public:
  VersionString(int major, int minor, int patch);
  VersionString(int major, int minor, int patch, const char* tag,
                int increment);
};

// =============================================================================
//                                 Parser
// =============================================================================

// Main class for parsing command line arguments.
/* Use `add_argument` to add actions (flags, positionals) to the parser, then
 * call `parse_args`. */
class Parser {
 public:
  // Collection of program metadata, used to initialize a parser.
  struct Metadata {
    bool add_help;               //< if true, add default `-h/--help` flags
    bool add_version;            //< if true, add default `-v/--version` flags
    std::string name;            //< the name of the program
    std::string version;         //< the program version number
    std::string author;          //< program author
    std::string copyright;       //< copyright statement
    std::string prolog;          //< help text printed before argument help
    std::string epilog;          //< help text printed after argument help
    std::string command_prefix;  //< used to forward down to subparsers
    size_t subdepth;             //< number of parsers between this one and the
                                 //  main one
  };

  // Construct a new parser.
  /* See the documentation for `Metadata` for the list of all optional
   * construction arguments */
  Parser(const Metadata& meta = {
             /*.add_help=*/true,
             /*.add_version=*/true});  // NOLINT(runtime/explicit)

  // Add a raw action as a flag
  template <typename T>
  void add_action(const std::string& short_flag, const std::string& long_flag,
                  const std::shared_ptr<Action<T>>& action);

  // Add a raw action as a positional or flag argument
  template <typename T>
  void add_action(const std::string& name_or_flag,
                  const std::shared_ptr<Action<T>>& action);

  // Add a flag argument with the given short and log flag names
  template <typename T = void>
  KWargs<T> add_argument(const std::string& short_flag,
                         const std::string& long_flag, KWargs<T> spec = {});

  // Add a flag argument with the given short and long flag names
  /* This template exists so that one can avoid specfying the template
     parameter explicitly. The template parameter <T> will be inferred by
     the type of the destination pointer.*/
  template <typename T>
  KWargs<typename ElementType<T>::value> add_argument(
      const std::string& short_flag, const std::string& long_flag, T* dest,
      KWargs<typename ElementType<T>::value> spec = {});

  // Add a positional argument or a flag argument that has either a short flag
  // or a long flag but not both.
  template <typename T = void>
  KWargs<T> add_argument(const std::string& name_or_flag, KWargs<T> spec = {});

  // Add a flag or positional argument with the given name
  /* This template exists so that one can avoid specfying the template
     parameter explicitly. The template parameter <T> will be inferred by
     the type of the destination pointer.*/
  template <typename T>
  KWargs<typename ElementType<T>::value> add_argument(
      const std::string& name_or_flag, T* dest,
      KWargs<typename ElementType<T>::value> spec = {});

  // Add a flag argument with the given short and long flag names using the
  // keywords API.
  template <TagNo TAG, class T, class... Args>
  void add_argument(const std::string& short_flag, const std::string& long_flag,
                    const KeywordArgument<TAG, T>& arg0, const Args&... args);

  // Add a flag argument with the given short and long flag names using the
  // keywords API.
  template <TagNo TAG, class T, class... Args>
  void add_argument(const std::string& name_or_flag,
                    const KeywordArgument<TAG, T>& arg0, const Args&... args);

  // Create the subparser action and return a handle to it. Use this handle
  // to add subparsers dispatched depending on the value of a string argument.
  std::shared_ptr<Subparsers> add_subparsers(const std::string& name,
                                             std::string* dest,
                                             const SubparserOptions& opts = {});

  // Parse command line out of a standard string vector, as expected in
  // `int main(int argc, char** argv)`.
  int parse_args(int argc, char** argv, std::ostream* log = &std::cerr);

  // Parse command line out of a list of strings. This is useful mostly for
  // testing/verification.
  int parse_args(const std::initializer_list<std::string>& init_list,
                 std::ostream* log = &std::cerr);

  // Parse command line arguments out of a list of string. Arguments are
  // removed from the list, modifying it as the parser works through it's
  // state machine.
  int parse_args(std::list<std::string>* args, std::ostream* log = &std::cerr);

  // Print the formatted usage string: the short specification usally printed
  // on command line error or at the top of the help output. This is the the
  // description of generally how to formulate a command call.
  void print_usage(std::ostream* out, size_t width = 80);

  // Return the formatted usage with default width as a string.
  std::string get_usage_string();

  // Collection of options for help printing
  struct HelpOptions {
    enum FormatNo { FORMAT_TEXT = 0, FORMAT_JSON };

    ColumnSpec columns;  //< specify output colums
    int depth;           //< depth of the print, for recursive cases
    FormatNo format;
    bool recurse;
  };

  // Print formatted multi-column help specification to the given stream.
  // This is the detailed description usually presented with `-h` or `--help`
  // that lists out all the command line options along with a sentence or
  // paragraph about what the option does.
  void print_help(std::ostream* out,
                  const HelpOptions& opts = {kDefaultColumns, 0});

  // Print the version string to the given stream;
  void print_version(std::ostream* out,
                     const ColumnSpec& columns = kDefaultColumns);

  // Backend for parse_args above. The only difference between the two is that//
  // parse_args() will swallow any exceptions thrown during parsing while this
  // function will allow them past.
  int parse_args_impl(std::list<std::string>* args,
                      const ParseContext& parent_ctx);

  // Match the current argument against the set of available flags or
  // positional arguments and output possible completions.
  int autocomplete(const ParseContext& parent_ctx);

  // Return the proglog for the parser help. Primarily used by subcommands for
  // subcommand indexing.
  std::string get_prolog(size_t column_width = 0);

  // Calls action->validate() for all positional and flag actions registered
  // to the parser.
  void validate();

 private:
  void print_helpText(std::ostream* out, const HelpOptions& opts);
  void print_helpJSON(std::ostream* out, const HelpOptions& opts);

  Metadata meta_;

  // Mapping of short flag strings (i.e. `-h` or `-v`) to the action associated
  // with them.
  std::map<std::string, FlagStore> short_flags_;
  std::map<std::string, FlagStore> short_flags_m_;  // mutable version

  // Mapping of long flag strings (i.e. `--help` or `--version`) to the action
  // associated with them.
  std::map<std::string, FlagStore> long_flags_;
  std::map<std::string, FlagStore> long_flags_m_;  // mutable version

  // List of actions associated with positional arguments.
  std::list<std::shared_ptr<ActionBase>> positionals_;
  std::list<std::shared_ptr<ActionBase>> positionals_m_;  // mutable version

  // A list of flag help specifications, in the order which the flags were
  // registered with the parser. This list is what is used by the printer
  // printing help output.
  std::list<FlagHelp> flag_help_;

  // A list of positional argument help specifications, in the order in which
  // the positional arguments where registered with the parser. This list is
  // used by the printer when printing help.
  std::list<PositionalHelp> positional_help_;

  // A list of subcommand help parsers so that we an recurse on sub commands
  std::list<std::shared_ptr<Subparsers>> subcommand_help_;

  // If a remainder action has been registered
  std::shared_ptr<ActionBase> remainder_action_;
};

}  // namespace argue
