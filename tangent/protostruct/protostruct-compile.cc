// Copyright 2022 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <glog/logging.h>

#include "argue/argue.h"
#include "tangent/protostruct/protostruct.h"
#include "tangent/util/exception.h"

int main(int argc, char** argv) {
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(argv[0]);

  argue::Parser::Metadata parser_meta;
  parser_meta.add_help = true;
  parser_meta.add_version = true;
  parser_meta.version = argue::VersionString TANGENT_PROTOSTRUCT_VERSION;
  parser_meta.author = "Josh Biakowski <josh.bialkowski@gmail.com>";
  argue::Parser argparser{parser_meta};
  ProgramOptions popts{};
  setup_parser_for_compile(&argparser, &popts);

  int parse_code = argparser.parse_args(argc, argv);
  switch (parse_code) {
    case argue::PARSE_FINISHED:
      break;
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
    default: {
      std::cerr << "Failed to parse command line";
      return 1;
    }
  }

  return compile_main(popts);
}
