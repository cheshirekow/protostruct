// Copyright 2021 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fstream>

#include "argue/argue.h"
#include "tangent/tjson/tjson.h"
#include "tangent/util/exception.h"

struct ProgramOpts {
  std::string command;
  std::string infile = "-";

  struct {
    bool omit_template;
  } markup;
};

std::ostream& operator<<(std::ostream& out,
                         const tjson_SourceLocation& location) {
  out << location.lineno << ":" << location.colno;
  return out;
}

std::ostream& operator<<(std::ostream& out, const tjson_Error& error) {
  out << "[" << error.code << "](" << error.loc << "): " << error.msg;
  return out;
}

int lex_file(const ProgramOpts& opts, const std::string& content) {
  tjson_Error error{};
  tjson_Scanner scanner;
  TANGENT_ASSERT(tjson_Scanner_init(&scanner, &error) == 0)
      << "Failed to initialize scanner: " << error;
  tjson_StringPiece content_piece = tjson_StringPiece_fromstr(content.c_str());
  TANGENT_ASSERT(tjson_Scanner_begin(&scanner, content_piece, &error) == 0)
      << "Failed to start scanner: " << error;

  tjson_Token token;
  uint32_t idx = 0;
  while (tjson_Scanner_pump(&scanner, &token, &error) == 0) {
    printf("%3d: [%14s](%d:%d) '%.*s'\n", idx++,
           tjson_TokenTypeNo_tostring(token.typeno), token.location.lineno,
           token.location.colno,
           static_cast<int>(tjson_StringPiece_size(token.spelling)),
           token.spelling.begin);
  }
  if (error.code == TJSON_LEX_INPUT_FINISHED) {
    return 0;
  } else {
    std::cerr << error.msg << "\n";
    return error.code;
  }
}

int parse_file(const ProgramOpts& opts, const std::string& content) {
  tjson_Error error{};
  tjson_LexerParser parser;
  TANGENT_ASSERT(tjson_LexerParser_init(&parser, &error) == 0)
      << "Failed to initialize parser: " << error;
  tjson_StringPiece content_piece = tjson_StringPiece_fromstr(content.c_str());
  TANGENT_ASSERT(tjson_LexerParser_begin(&parser, content_piece, &error) == 0)
      << "Failed to start parsing: " << error;

  tjson_Event event;
  uint32_t idx = 0;
  while (tjson_LexerParser_get_next_event(&parser, &event, &error) == 0) {
    printf("%3d: [%13s] '%.*s'\n", idx++,
           tjson_EventTypeNo_tostring(event.typeno),
           static_cast<int>(tjson_StringPiece_size(event.token.spelling)),
           event.token.spelling.begin);
  }
  if (error.code == TJSON_LEX_INPUT_FINISHED) {
    return 0;
  } else {
    std::cerr << error.msg << "\n";
    return error.code;
  }
}

const char* kProlog =
    "Demonstrates the usage of the json library to lex and parse JSON data";

int main(int argc, char** argv) {
  argue::Parser::Metadata meta{};
  meta.add_help = true;
  meta.add_version = true;
  meta.name = "json";
  meta.version = argue::VersionString TJSON_VERSION;
  meta.author = "Josh Bialkowski <josh.bialkowski@gmail.com>";
  meta.copyright = "(C) 2021";
  meta.prolog = kProlog;

  argue::Parser parser(meta);

  ProgramOpts opts;
  auto subparsers = parser.add_subparsers(
      "command", &opts.command,
      {.help = "Each subcommand has it's own options and arguments, see "
               "individual subcommand help."});
  auto lex_parser = subparsers->add_parser(
      "lex", {.help = "Lex the file and dump token information"});
  auto parse_parser = subparsers->add_parser(
      "parse", {.help = "Parse the file and dump actionable parse events"});

  for (auto& subparser : {lex_parser, parse_parser}) {
    argue::KWargs<std::string> kwargs{
        //
        .action = "store",  .nargs = "?",
        .const_ = {},       .default_ = std::string("-"),
        .choices = {},      .dest = {},
        .required = false,  .help = "Path to input, '-' for stdin",
        .metavar = "infile"};

    subparser->add_argument(  //
        "infile", &opts.infile,
        {.action = "store",
         .nargs = "?",
         .const_ = {},
         .default_ = std::string("-"),
         .choices = {},
         .dest = {},
         .required = false,
         .help = "Path to input, '-' for stdin",
         .metavar = "infile"});
  }

  int parse_result = parser.parse_args(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  std::istream* infile;
  std::ifstream open_infile;
  if (opts.infile == "-") {
    infile = &std::cin;
  } else {
    open_infile.open(opts.infile);
    infile = &open_infile;
  }

  std::string content;
  content.reserve(1024 * 1024);
  content.assign((std::istreambuf_iterator<char>(*infile)),
                 std::istreambuf_iterator<char>());

  if (opts.command == "lex") {
    exit(lex_file(opts, content));
  } else if (opts.command == "parse") {
    exit(parse_file(opts, content));
  } else {
    printf("Unknown command: %s\n", opts.command.c_str());
  }
  return 0;
}
