// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/argue.h"

struct ProgramOptions {
  struct Foo {
    std::string arg1;
    std::string arg2;
    std::string option1;
    std::string option2;
  } foo;

  struct Bar {
    std::string option1;
    std::string option2;
  } bar;

  std::string command;
};

int main(int argc, char** argv) {
  argue::Parser::Metadata meta{};
  meta.add_help = true;
  meta.name = "subparser-example";
  argue::Parser parser{meta};

  ProgramOptions progopts{};
  auto subparsers = parser.add_subparsers("command", &progopts.command);

  auto foo_parser =
      subparsers->add_parser("foo", {.help = "The foo command does foo"});
  {
    auto spec = foo_parser->add_argument("arg1", &progopts.foo.arg1);
    spec.help = "First positional argument is mandatory";
  }
  {
    auto spec = foo_parser->add_argument("arg2", &progopts.foo.arg2);
    spec.nargs = "?";
    spec.help = "Second positional argument is optional";
  }
  foo_parser->add_argument("--foo-opt1", &progopts.foo.option1);
  foo_parser->add_argument("--foo-opt2", &progopts.foo.option2);

  auto bar_parser =
      subparsers->add_parser("bar", {.help = "The bar command does bar"});
  bar_parser->add_argument("--bar-opt1", &progopts.bar.option1);
  bar_parser->add_argument("--bar-opt2", &progopts.bar.option2);

  int parse_result = parser.parse_args(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  std::cout << progopts.command << "\n";
  return 0;
}
