// Copyright 2022 Josh Bialkowski <josh.bialkowski@gmail.com>
#pragma once

#include <string>
#include <vector>

#include "argue/argue.h"

#define TANGENT_PROTOSTRUCT_VERSION \
  { 0, 2, 0, "dev", 5 }

struct ProgramOptions {
  std::string command;

  /// List of directories to include on the proto-path
  std::vector<std::string> proto_path;

  /// List of uncompiled .proto files to accept as input for either
  /// compile or gen
  std::vector<std::string> proto_inpaths;

  /// Path to a binary encoded FileDescriptorSet containing input
  /// descriptors to use for synchronization
  std::string descriptor_set_inpath;

  struct CompileOptions {
    /// Path to the input file to compile. May be a source file or a header
    std::string source_filepath;

    /// Where to write the output. The output is a binary encoded
    /// FileDescriptorSet.
    std::string binary_outpath;

    /// If specified, these filters are used to restrict
    /// which header files are processed for message definitions. Each entry
    /// is a (possibly negated) regex pattern. A struct/enum will be processed
    /// if the header containing it matches an inclusion pattern. Patterns are
    /// matched in order with the latest matching pattern taking priority.
    /// If not specified, then only those defined in source_filepath are
    /// processed.
    std::vector<std::string> source_patterns;

    /// If specified, these are used to restrict which messages and enums are
    /// included in the generated descriptors. Each entry is a regex pattern.
    /// a struct or enum is included if matches an inclusion pattern and is
    /// not excluded by a later exclusion pattern.
    std::vector<std::string> name_patterns;

    /// List of flags passed to libclang to build the translation unit (e.g.
    /// compilation flags).
    std::vector<std::string> clang_options;
  } compile_opts;

  struct GenerateOptions {
    /// Root directory where to generate C++ language bindings. Files are
    /// written to a relative path matching the .proto from which they are
    /// generated
    std::string cpp_root;

    /// List of templates to use for generation
    std::vector<std::string> templates;
  } gen_opts;
};

void setup_parser_for_compile(argue::Parser* parser, ProgramOptions* opts);
void setup_parser(argue::Parser* parser, ProgramOptions* opts);
void compile_descriptor_set(const ProgramOptions& popts);
int compile_main(const ProgramOptions& popts);
int gen_main(const ProgramOptions& popts);
