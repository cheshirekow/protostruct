#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "argue/storage_model.h"

namespace argue {

// =============================================================================
//                              Actions
// =============================================================================

class Parser;

struct AutoCompleteContext {
  bool active{false};  //< True if we are in autocomplete mode
  std::list<std::string>::iterator
      comp_word;    //< pointer to the word in the list that needs completion
  std::string ifs;  //< bash array separator
  std::ostream* debug;  //< debug log
};

// Context provided to Action objects during argument parsing
struct ParseContext {
  Parser* parser;     //< Pointer to the parser that owns the argument
  std::ostream* out;  //< output stream where to write any messages
  std::string arg;    //< the argument that initiated this action, in the case
                      //  of actions associated with flags. Empty for
                      //  actions associated with positionals
  AutoCompleteContext auto_complete;
};

// Enumerates the possible result cases from a call to `parse_args`.
enum ParseResult {
  PARSE_FINISHED = 0,   // Finished parsing arguments, no errors
  PARSE_ABORTED = 1,    // Parsing was terminated early but not in error,
                        // usually this means --help or --version
  PARSE_EXCEPTION = 2,  // An exception occured and parsing failed
  // TODO(josh): add separate cases for USER exception and DEVELOPER exception,
  // so that we know what to print on the console.
};

// Context filled by an action at it's conclusion
struct ActionResult {
  bool keep_active;  //< Set true by the action if it wishes to remain active
                     //  after processing. This is only meaningful for flags
                     //  and will be ignored for positionals.
  ParseResult code;  //< success/failure of the parse
};

// Indicates what kind of argument a particular action is associated with
enum Usage {
  USAGE_POSITIONAL = 0,  //< action is associated with a positional
  USAGE_FLAG             //< action is associated with a flag
};

// Interface shared by all action objects.
class ActionBase {
 public:
  ActionBase();
  virtual ~ActionBase();

  // Set the number of arguments accepted by the action.
  virtual void set_nargs(int nargs);

  // Set whether or not the argument is required. Note that assignment of this
  // value is only meaningful for actions associated with flags.
  virtual void set_required(bool required);

  // Set the help text used to describe the action
  virtual void set_help(const std::string& help);

  // Set the metavariable string used to placehold the argument values when
  // constructing help text
  virtual void set_metavar(const std::string& metavar);

  // Set the usage (flag or positional) that the action is associated with
  virtual void set_usage(Usage usage);

  // Return true if the action is fully and correctly configured.
  /* This is the best place to implement assertions regarding the configuration
   * of the action as they'll be caugh regardless of what command line arguments
   * are pumped through the parser.
   *
   * `Valdate()` has the side effect of assigning default values wherever
   * they have been configured.
   *
   * TODO(josh): move default assignment to a separate function! */
  virtual bool validate();

  // Return true if the argument is required.
  /* This is used after all arguments are consumed to determine if the command
   * line was valid. If any arguments remain in the queue that are marked
   * required, then the parse fails.
   *
   * In general, an argument is required if:
   *  * it is a positional with nargs other than '+' or '*'
   *  * it is a flag and `{.required=true}` was specified
   * */
  virtual bool is_required() const;

  // Return the number of arguments consumed by this action.
  // TODO(josh): remove ``default_value``, just set the default value in the
  // constructor!
  virtual int get_nargs(int default_value) const;

  // Return the string used to represent the value of this argument in help
  // text.
  // TODO(josh): remove ``default_value``, just set the default in the
  // constructor!
  virtual std::string get_metavar(const std::string& default_value) const;

  // Return right hand side of help text for the help table
  /* Generally this is composed of whatever was provided by `{.help=}` during
   * a call to `add_argument`. However it will also include some generated
   * content like `choices` or `default` */
  virtual std::string get_help(size_t column_width = 0) const;

  // Parse zero or more argument values out of the list `args`
  /* Actions should modify args and leave it in a state consistent with
   * "remaining arguments". */
  virtual void consume_args(const ParseContext& ctx,
                            std::list<std::string>* args,
                            ActionResult* result) = 0;

  // Assuming that this action were next in the parse queue (e.g. consume_args
  // would have been called given the argument list up to this point), then
  // match the current argument against available arguments (e.g. choices,
  // subcommands, etc).
  virtual void write_completions(const ParseContext& ctx) {}

  // Assign the parser that this action is attached to. If this action has
  // already been assigned to a parser, then throw an exception.
  void set_parser(Parser* parser);

 protected:
  std::string type_name_;

  uint32_t usage_ : 1;            //< USAGE_FLAGS or USAGE_POSITIONAL
  uint32_t has_nargs_ : 1;        //< true if nargs_ has been assigned
  uint32_t has_const_ : 1;        //< true if const_ has been assigned
  uint32_t has_default_ : 1;      //< true if default_ has been assigned
  uint32_t has_choices_ : 1;      //< true if choices_ has been assigned
  uint32_t has_required_ : 1;     //< true if required_ has been assigned
  uint32_t has_help_ : 1;         //< true if help_ has been assigned
  uint32_t has_metavar_ : 1;      //< true if metavar_ has been assigned
  uint32_t has_destination_ : 1;  //< true if destination_ has been assigned

  int nargs_;              //< number of arguments consumed by this action
  bool required_ = false;  //< true if this action is required, and if it should
                           //  be considered an error if this action remains
                           //  after all arguments are consumed
  std::string help_;       //< help text for this action
  std::string metavar_;    //< string to use in place of this actions values
                           //  when constructing usage or help text

  Parser* parser_;  //< The parser that this action has been assigned to
};

// Interface shared by all action objects that support the standard setters.
/* TODO(josh): change name of ActionBase to Action and change name of
 * Action to StandardAction */
template <typename T>
class Action : public ActionBase {
 public:
  Action();
  virtual ~Action() {}

  virtual void set_const(const T& value);

  void set_default(const T& value);
  void set_default(const std::vector<T>&& value);

  void set_choices(const std::vector<T>&& value);
  void set_choices(const std::vector<T>& value);
  void set_choices(const std::initializer_list<T>& value);

  void set_destination(const std::shared_ptr<StorageModel<T>>& destination);
  void set_destination(T* destination);

  template <class Allocator>
  void set_destination(std::list<T, Allocator>* destination);

  template <class Allocator>
  void set_destination(std::vector<T, Allocator>* destination);

 protected:
  // A list of the valid values allowed to be consumed by this action
  std::vector<T> choices_;

  // default list of values used to initialize the destination at the start of
  // parsing, if configured
  std::vector<T> default_;

  // pointer to the object which is filled during `consume_args`
  std::shared_ptr<StorageModel<T>> destination_;
};

// Specialization for actions that don't support standard setters for data
/* Actions of this type still support the non-template setters like `SetHelp()`
 * or `SetMetavar` */
template <>
class Action<void> : public ActionBase {
 public:
  Action() {}
  virtual ~Action() {}
};

// Implements the "store" action
/* The "store" simply parses the string into a value of the correct
 * type and passes it to the storage model. This includes both list-like
 * destinations and scalar destinations. */
template <typename T>
class StoreValue : public Action<T> {
 public:
  virtual ~StoreValue() {}

  bool is_scalar() const;
  std::string get_help(size_t column_width) const override;
  bool validate() override;
  void consume_args(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result) override;

 protected:
  void consume_scalar(const ParseContext& ctx, std::list<std::string>* args,
                      ActionResult* result);
  void consume_list(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result);
};

// Implements the "store_const" action for scalars.
/* The "store_const" action simply copies some "constant" value into the
 * destination when activated. It does not remove any arguments from the
 * argument list during `consume_args`. */
template <typename T>
class StoreConst : public StoreValue<T> {
 public:
  virtual ~StoreConst() {}
  void set_const(const T& value) override;

  bool validate() override;
  void consume_args(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result) override;

 protected:
  // value which is assigned to the `destination_` when this action is
  // processed (via `consume_args`).
  std::vector<T> const_;
};

// Implements the "append" action
/* The "append" action simply parses the string into a value of the correct
 * type and passes it to the storage model. This is valid only for list-like
 * destinations. */
template <typename T>
class AppendValue : public Action<T> {
 public:
  virtual ~AppendValue() {}

  std::string get_help(size_t column_width) const override;
  bool validate() override;
  void consume_args(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result) override;

 protected:
  void consume_scalar(const ParseContext& ctx, std::list<std::string>* args,
                      ActionResult* result);
  void consume_list(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result);
};

// Implements the "help" action, which prints help text and terminates the
// parse loop.
class Help : public Action<void> {
 public:
  virtual ~Help() {}
  std::string get_help(size_t column_width) const override;
  bool validate() override;
  void consume_args(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result) override;
};

// Implements the "version" action, which prints version text and terminates
// the parse loop.
class Version : public Action<void> {
 public:
  virtual ~Version() {}
  std::string get_help(size_t column_width) const override;
  bool validate() override;
  void consume_args(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result) override;
};

// Optional parameters provided to `add_subparsers`.
struct SubparserOptions {
  std::string help;  //< help text used to describe the subparsers
};

// Implements the subparser action, which dispatches another parser
/* Subparser actions act like the "store" action for a string type, in the
 * sense that they consume one argument (the subcommand) and store it in a
 * string. They internally store a map of `string` (command name) -> `Parser`
 * and when activated they call `parse_args` on whatever `Parser` was mapped
 * to the provided command. They pass the remaining argument list to the
 * subparser.
 *
 * Note that, unlike other actions, this object is returned by the `Parser`
 * that owns it, so that `AddParser` can be called by the user.
 */
class Subparsers : public StoreValue<std::string> {
 public:
  // The type of the map that we store, mapping comman names to subparsers
  typedef std::map<std::string, std::shared_ptr<Parser>> MapType;

  // These options are cached by the `Subparsers` object and passed on to
  // each parser which it constructs.
  struct Metadata {
    std::string command_prefix;  //< this string is appended to the start of
                                 //  the usage description. It is intended to
                                 //  contain the command string up to this sub
                                 //  command

    size_t subdepth;  //< how many subparsers exist between this one
                      //  and the main one
  };

  explicit Subparsers(const Metadata& meta = {});
  virtual ~Subparsers() {}

  // Recursively validate this action and all subparsers
  bool validate() override;

  std::string get_help(size_t column_width) const override;

  // Consume one argument, pass remaining args to appropriate subparser
  /* This action will consume one argument from the argument list. If that
   * argument matches a `command` in the subparser map, then it will pass
   * the remaining arguments to the subparser. Otherwise it is an error and
   * it will throw. */
  void consume_args(const ParseContext& ctx, std::list<std::string>* args,
                    ActionResult* result) override;

  // Convenience accessor to subparser map iterator
  MapType::const_iterator begin() const;

  // Convenience accessor to subparser map iterator
  MapType::const_iterator end() const;

  // Add a new subparser associated with the given command
  /* This call will construct a new parser object using the provided options,
   * and it will store a pointer to that parser in a map associating it with
   * `command`.
   *
   * When this action is activated it will consume one argument from the
   * argument list. If that argument matches `command`, then it will pass
   * the remaining arguments to the subparser. */
  std::shared_ptr<Parser> add_parser(const std::string& command,
                                     const SubparserOptions& opts = {});

  void write_completions(const ParseContext& ctx) override;

 private:
  MapType subparser_map_;  //< maps command names to parser objects
  Metadata metadata_;      //< cache of common options used for all subparsers
                           // constructed
};

}  // namespace argue
