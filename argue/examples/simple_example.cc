// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "argue/argue.h"

int main(int argc, char** argv) {
  argue::Parser::Metadata meta{/*add_help=*/true};
  meta.name = "subparser-example";
  argue::Parser parser{meta};

  std::string foo;
  std::string bar;
  parser.add_argument("arg", &foo);
  parser.add_argument("-b", "--bar", &bar);

  int parse_result = parser.parse_args(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  std::cout << "foo: " << foo << ", bar: " << bar << std::endl;
  return 0;
}
