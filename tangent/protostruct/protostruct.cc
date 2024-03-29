// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <Python.h>

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <clang-c/Index.h>
#include <glog/logging.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/wire_format_lite.h>

#include "argue/argue.h"
#include "tangent/protostruct/descriptor_extensions.pb.h"
#include "tangent/protostruct/protostruct.h"
#include "tangent/util/exception.h"
#include "tangent/util/fallthrough.h"
#include "tangent/util/stringutil.h"

// NOTE(josh): to self, see:
// https://developers.google.com/protocol-buffers/docs/reference/cpp#google.protobuf.compiler
// for c++ API to read .proto.

// Convert a clang string to a C++ string and then release the held clang
// reference to that string
std::string drop_cxstring(CXString clang_string) {
  const char* result_cstr = clang_getCString(clang_string);
  std::string result;
  if (result_cstr) {
    result = result_cstr;
  } else {
    result = "<null>";
  }
  clang_disposeString(clang_string);
  return result;
}

class MyErrorCollector
    : public google::protobuf::compiler::MultiFileErrorCollector {
 public:
  MyErrorCollector() : status_{0} {}
  virtual ~MyErrorCollector() {}
  void AddError(const std::string& filename, int line, int column,
                const std::string& message) override {
    std::cerr << filename << ":" << line << "," << column << ": " << message
              << "\n";
    status_ = 1;
  }
  void AddWarning(const std::string& filename, int line, int column,
                  const std::string& message) override {
    std::cerr << filename << ":" << line << "," << column << ": " << message
              << "\n";
  }

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(MyErrorCollector);

  int status_;
};

std::ostream& operator<<(std::ostream& stream, const CXString& str) {
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

std::ostream& operator<<(std::ostream& stream,
                         const CXSourceLocation& location) {
  CXFile file{};
  unsigned int lineno{0};
  unsigned int colno{0};
  clang_getSpellingLocation(location, &file, &lineno, &colno, nullptr);
  CXString filename_cxstr = clang_getFileName(file);
  stream << filename_cxstr << ":" << lineno << "," << colno;
  return stream;
}

struct NameParts {
  std::string reldir;
  std::string basename;
};

/// Split a filepath into a directory part and basename
NameParts splitpath(const std::string& filepath) {
  NameParts out{};
  auto parts = stringutil::split(filepath, '/');
  std::string filename = parts.back();
  parts.pop_back();
  out.reldir = stringutil::join(parts, "/");
  parts = stringutil::split(filename, '.');
  out.basename = parts[0];
  return out;
}

/// Construct a filepath from the given pattern by replacing {reldir} and
/// {basename} of `filepath`
std::string substitute_path_pattern(const std::string& pattern,
                                    const std::string& filepath) {
  auto name_parts = splitpath(filepath);
  std::string result = pattern;
  result = stringutil::replace(result, "{reldir}", name_parts.reldir);
  result = stringutil::replace(result, "{basename}", name_parts.basename);
  return result;
}

template <class T, class U>
bool operator==(const google::protobuf::RepeatedField<T>& lhs,
                const std::vector<U>& rhs) {
  if (lhs.size() != static_cast<int>(rhs.size())) {
    return false;
  }
  for (int idx = 0; idx < lhs.size(); idx++) {
    if (lhs.Get(idx) != rhs[idx]) {
      return false;
    }
  }
  return true;
}

google::protobuf::SourceCodeInfo_Location* find_location(
    google::protobuf::FileDescriptorProto* proto,
    const std::vector<int>& query_path) {
  auto* source_code_info = proto->mutable_source_code_info();
  for (int idx = 0; idx < source_code_info->location_size(); idx++) {
    auto* location = source_code_info->mutable_location(idx);
    if (location->path() == query_path) {
      return location;
    }
  }

  auto* location = source_code_info->add_location();
  for (size_t idx = 0; idx < query_path.size(); idx++) {
    location->add_path(query_path[idx]);
  }
  return location;
}

std::string get_cursor_comment(CXCursor c) {
  std::string comment;
  CXString comment_cxstr = clang_Cursor_getRawCommentText(c);
  const char* comment_cstr = clang_getCString(comment_cxstr);
  if (comment_cstr) {
    comment = clang_getCString(comment_cxstr);
  }
  clang_disposeString(comment_cxstr);
  return comment;
}

std::string strip_comment_prefix(const std::string& comment) {
  std::stringstream strm{};
  if (stringutil::startswith(comment, "/*")) {
    for (auto& line : stringutil::split(comment, '\n')) {
      if (stringutil::startswith(line, "/*")) {
        strm << line.substr(2);
      } else if (stringutil::startswith(line, "*")) {
        strm << line.substr(1);
      } else if (stringutil::startswith(line, " *")) {
        strm << line.substr(2);
      } else {
        strm << line;
      }
    }
    std::string out = strm.str();
    return out.substr(0, out.size() - 2);
  } else {
    for (auto& line : stringutil::split(comment, '\n')) {
      if (stringutil::startswith(line, "//!<")) {
        strm << line.substr(4);
      } else if (stringutil::startswith(line, "///<")) {
        strm << line.substr(4);
      } else if (stringutil::startswith(line, "///")) {
        strm << line.substr(3);
      } else if (stringutil::startswith(line, "//<")) {
        strm << line.substr(3);
      } else if (stringutil::startswith(line, "//")) {
        strm << line.substr(2);
      }
    }
    return strm.str();
  }
}

void setup_common_options(argue::Parser* parser, ProgramOptions* opts) {
  using argue::keywords::action;
  using argue::keywords::dest;
  using argue::keywords::help;

  // clang-format off
  parser->add_argument(
      "--proto-path", action="append", dest=&(opts->proto_path),
      help="Root directory of the search tree for dependant protocol buffer."
           " definitions. Can be specified multiple times.");

  parser->add_argument(
      "--proto-in", action="append", dest=&(opts->proto_inpaths),
      help="List of .proto files to compile into a FileDescriptorSet as input"
           " to either `compile` or `gen");

  parser->add_argument(
    "--descriptor-set-in", dest=&(opts->descriptor_set_inpath),
    help="If provided, this binary serializaiton of a FileDescriptorSet is"
          " parsed and used to initialize each FileDescriptor which is"
          " generated. This allows you to 'synchronize' between an a set of"
          " .proto's and .h's");
  // clang-format on
}

void setup_compile_options(argue::Parser* compile_parser,
                           ProgramOptions* opts) {
  using argue::keywords::dest;
  using argue::keywords::help;
  using argue::keywords::nargs;

  // clang-format off
  compile_parser->add_argument(
      "sourcefile", dest=&(opts->compile_opts.source_filepath), nargs='?',
      help="The source file to compile");

  compile_parser->add_argument(
      "-o", "--pb3-out", dest=&(opts->compile_opts.binary_outpath),
      help="The output file were we write the binary serialization of the "
            "generated FileDescriptorSet");

  compile_parser->add_argument(
      "-s", "--source-patterns", dest=&(opts->compile_opts.source_patterns),
      nargs="+",
      help = "If specified, these filters are used to restrict which header "
      "files are processed for message definitions. Each entry is a "
      "(possibly negated) regex pattern. A struct/enum will be processed if "
      "the header containing it matches an inclusion pattern. Patterns are "
      "matched in order with the latest matching pattern taking priority. If "
      "not specified, then only those defined in source_filepath are "
      "processed.");

  compile_parser->add_argument(
      "-n", "--name-patterns", dest=&(opts->compile_opts.name_patterns),
      nargs="+",
      help="If specified, these are used to restrict which messages and enums"
      " are included in the generated descriptors. Each entry is a regex"
      " pattern. A struct or enum is included if matches an inclusion pattern"
      " and is not excluded by a later exclusion pattern.");

  compile_parser->add_argument(
    "remainder", nargs=argue::REMAINDER,
    dest=&(opts->compile_opts.clang_options),
    help="Command line flags passed to clang");
  // clang-format on
}

void setup_parser(argue::Parser* parser, ProgramOptions* opts) {
  using argue::keywords::action;
  using argue::keywords::choices;
  using argue::keywords::dest;
  using argue::keywords::help;
  using argue::keywords::nargs;

  setup_common_options(parser, opts);

  // clang-format off
  auto subparsers = parser->add_subparsers(
      "command", &opts->command, {
        .help="`compile` a proto description from an existing header, or `gen`"
        " bindings from a proto description"});

  auto compile_parser = subparsers->add_parser(
      "compile", {.help="compile a proto description from an existing header"});

  setup_compile_options(compile_parser.get(), opts);

  auto gen_parser = subparsers->add_parser(
      "generate", {.help="generate bindings from a protobuf descriptor"});

  gen_parser->add_argument(
      "--cpp-root", dest=&(opts->gen_opts.cpp_root),
      help="Root directory of the output tree for generated C and C++ files");

  gen_parser->add_argument(
      "templates", dest=&(opts->gen_opts.templates), nargs="+",
      choices={"cpp-simple", "cereal", "pb2c", "pbwire", "proto", "recon"},
      help="Generate bindings from these templates");
  // clang-format on
}

void setup_parser_for_compile(argue::Parser* parser, ProgramOptions* opts) {
  setup_common_options(parser, opts);
  setup_compile_options(parser, opts);
}

/// Base class for different visitors. It allows us to use polymorphism for
/// improved type safety and readabilty when dispatching visitors through
/// clangs C-API. It works together with visit_tree, below, to conveniently
/// dispatch one of our visitors on a subtree of the AST.
class ClangVisitor {
 public:
  /// This is the (virtual) member which does the actual work. It does not
  /// include a "client_data" pointer because that pointer is the "this"
  /// pointer of this object.
  virtual CXChildVisitResult visit(CXCursor c, CXCursor parent) = 0;

  /// Called by visit_tree at the conclusion of the visitation. Can be used
  /// to do any "flush" work, depending on the visitor.
  virtual void finish_visit() {}

  /// This is the callback supplied to the clang visitChildren() function. It
  /// simply casts the untyped "client_data" to a pointer of this class and then
  /// calls the visit() member function.
  static CXChildVisitResult Callback(CXCursor c, CXCursor parent,
                                     CXClientData client_data) {
    return reinterpret_cast<ClangVisitor*>(client_data)->visit(c, parent);
  }
};

/// Dispatch a visitor of any type on the AST subtree described by `cursor`.
template <class VisitorType, class... Args>
void visit_tree(CXCursor cursor, Args&&... args) {
  VisitorType visitor{std::forward<Args>(args)...};
  clang_visitChildren(cursor, ClangVisitor::Callback, &visitor);
  visitor.finish_visit();
}

class DebugVisitor : public ClangVisitor {
 public:
  DebugVisitor() {}
  virtual ~DebugVisitor() {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    LOG(WARNING) << "DebugVisitor::HERE";
    CXCursorKind kind = clang_getCursorKind(c);
    LOG(WARNING) << clang_getCursorKindSpelling(kind);
    return CXChildVisit_Recurse;
  }
};

/// Visitor for enumeration values. Ignores all cursors other than
/// EnumConstantDecl, and in those cases add a new value to the currently active
/// enumeration in the descriptor we are constructing.
class EnumVisitor : public ClangVisitor {
 public:
  explicit EnumVisitor(google::protobuf::EnumDescriptorProto* proto,
                       int enumidx,
                       google::protobuf::FileDescriptorProto* file_proto)
      : proto_{proto}, enumidx_{enumidx}, file_proto_{file_proto} {
    proto_->clear_value();
  }
  virtual ~EnumVisitor() {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    CXCursorKind kind = clang_getCursorKind(c);
    if (kind != CXCursor_EnumConstantDecl) {
      return CXChildVisit_Continue;
    }

    int value_idx = proto_->value_size();
    auto* value = proto_->add_value();
    CXString namestr = clang_getCursorSpelling(c);
    value->set_name(clang_getCString(namestr));
    clang_disposeString(namestr);

    const int number = clang_getEnumConstantDeclValue(c);
    value->set_number(number);

    CXString comment_cxstr = clang_Cursor_getRawCommentText(c);
    const char* comment_cstr = clang_getCString(comment_cxstr);
    if (comment_cstr) {
      std::string comment = clang_getCString(comment_cxstr);
      clang_disposeString(comment_cxstr);

      value->mutable_options()
          ->MutableExtension(protostruct::enumvopts)
          ->set_comment(comment);

      //  enum_type is field #5 within FileDescriptorProto
      auto* location = find_location(file_proto_, {5, enumidx_, 2, value_idx});
      std::string stripped_comment = strip_comment_prefix(comment);

      // TODO(josh): this is a hack, use clang_Cursor_getCommentRange to
      // determine if it's leading or trailing
      if (stringutil::startswith(comment, "//!<") ||
          stringutil::startswith(comment, "///<")) {
        location->set_trailing_comments(stripped_comment);
      } else {
        location->set_leading_comments(stripped_comment);
      }
    }

    // find the value in the existing proto
    return CXChildVisit_Continue;
  }

 private:
  google::protobuf::EnumDescriptorProto* proto_;
  int enumidx_;
  google::protobuf::FileDescriptorProto* file_proto_;
};

/// If `needle` is an enumeration already existing in `proto`, then return
/// a mutable pointer to it. Otherwise, create it in `proto`, and return
/// a mutable pointer to the new enuemration.
google::protobuf::EnumDescriptorProto* find_enum(
    google::protobuf::FileDescriptorProto* proto, std::string needle,
    int* found_idx) {
  std::string needle_stripped;
  // TODO(josh): make this generic
  if (needle.substr(0, 2) == "dw") {
    needle_stripped = needle.substr(2);
  } else {
    needle_stripped = needle;
  }

  for (int idx = 0; idx < proto->enum_type_size(); idx++) {
    google::protobuf::EnumDescriptorProto* candidate_proto =
        proto->mutable_enum_type(idx);
    std::string candidate_name = candidate_proto->name();
    std::string candidate_name_stripped;
    // TODO(josh): make this generic
    if (candidate_name.substr(0, 2) == "dw") {
      candidate_name_stripped = candidate_name.substr(2);
    } else {
      candidate_name_stripped = candidate_name;
    }

    if (stringutil::to_lower(candidate_name_stripped) ==
        stringutil::to_lower(needle_stripped)) {
      if (candidate_name != needle) {
        LOG(WARNING) << "Enum " << candidate_name << " changed to " << needle;
        candidate_proto->set_name(needle);
      }
      *found_idx = idx;
      return candidate_proto;
    }
  }

  *found_idx = proto->enum_type_size();
  auto* new_enum = proto->add_enum_type();
  new_enum->set_name(needle);
  return new_enum;
}

void get_compatible(
    CXType field_type, google::protobuf::FieldDescriptorProto* proto,
    std::vector<google::protobuf::FieldDescriptorProto_Type>* out) {
  out->clear();
  switch (field_type.kind) {
    case CXType_Float:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_FLOAT);
      TANGENT_FALLTHROUGH
    case CXType_Double:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_DOUBLE);
      return;

    case CXType_SChar:
      TANGENT_FALLTHROUGH
    case CXType_Short:
      TANGENT_FALLTHROUGH
    case CXType_Int:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_INT32);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_SINT32);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_FIXED32);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_SFIXED32);
      TANGENT_FALLTHROUGH
    case CXType_Long:
    case CXType_LongLong:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_INT64);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_SINT64);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_FIXED64);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_SFIXED64);
      return;

    case CXType_UChar:
      TANGENT_FALLTHROUGH
    case CXType_UShort:
      TANGENT_FALLTHROUGH
    case CXType_UInt:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_UINT32);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_INT32);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_FIXED32);
      TANGENT_FALLTHROUGH
    case CXType_ULong:
    case CXType_ULongLong:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_UINT64);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_INT64);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_FIXED64);
      return;

    case CXType_Bool:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_BOOL);
      return;

    default:
      break;
  }
}

int set_default_unless_already_compatible(
    google::protobuf::FieldDescriptorProto* proto, CXType field_type) {
  std::vector<google::protobuf::FieldDescriptorProto_Type> compatible{};
  get_compatible(field_type, proto, &compatible);
  if (compatible.size() < 1) {
    return 1;
  }

  if (!proto->has_type()) {
    proto->set_type(compatible[0]);
  }

  if (proto->type() == compatible[0]) {
    // return without log if already matches default
    return 0;
  }

  for (auto value : compatible) {
    if (value == proto->type()) {
      LOG(INFO) << "Preserving compatible "
                << google::protobuf::FieldDescriptorProto_Type_Name(
                       proto->type())
                << " for `" << proto->name() << "` ("
                << clang_getTypeSpelling(field_type) << ")";
      return 0;
    }
  }

  proto->set_type(compatible[0]);
  return 0;
}

std::map<std::string, std::string> STDINT_CONVERSIONS = {
    {"signed char", "int8_t"},
    {"short", "int16_t"},
    {"unsigned char", "uint8_t"},
    {"unsigned short", "uint16_t"},
    {"_Bool", "bool"}};

int set_field_type(google::protobuf::FieldDescriptorProto* proto,
                   CXType field_type) {
  auto* my_options =
      proto->mutable_options()->MutableExtension(protostruct::fieldopts);

  switch (field_type.kind) {
    case CXType_SChar:
    case CXType_Short:
    case CXType_UChar:
    case CXType_UShort: {
      std::string type_spelling =
          drop_cxstring(clang_getTypeSpelling(field_type));
      auto iter = STDINT_CONVERSIONS.find(type_spelling);
      if (iter != STDINT_CONVERSIONS.end()) {
        type_spelling = iter->second;
      }
      my_options->set_fieldtype(type_spelling);
    }
      TANGENT_FALLTHROUGH
    case CXType_Int:
    case CXType_LongLong:
    case CXType_Long:
    case CXType_UInt:
    case CXType_ULongLong:
    case CXType_ULong:
    case CXType_Float:
    case CXType_Double: {
      return set_default_unless_already_compatible(proto, field_type);
    }

    case CXType_Typedef: {
      CXCursor decl = clang_getTypeDeclaration(field_type);
      CXType underlying_type = clang_getTypedefDeclUnderlyingType(decl);
      // LOG(INFO) << "typedef " << clang_getTypeSpelling(field_type) << " -> "
      //           << clang_getTypeSpelling(underlying_type);
      return set_field_type(proto, underlying_type);
    }
    case CXType_Elaborated: {
      // TODO(josh): need to deal with anonymous enums and structs. They
      // somehow end up resolving to int (which I think is the default type
      // if no type can be resolved).
      CXType named_type = clang_Type_getNamedType(field_type);
      return set_field_type(proto, named_type);
    }
    case CXType_Record: {
      CXCursor decl = clang_getTypeDeclaration(field_type);
      proto->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_MESSAGE);
      CXString typename_cxstr = clang_getCursorSpelling(decl);
      proto->set_type_name(clang_getCString(typename_cxstr));
      clang_disposeString(typename_cxstr);
      return 0;
    }
    case CXType_Enum: {
      // CXCursor decl = clang_getTypeDeclaration(field_type);
      // CXString typename_cxstr = clang_getCursorSpelling(decl);
      proto->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_ENUM);
      CXString typename_cxstr = clang_getTypeSpelling(field_type);
      std::string typename_str =
          stringutil::pop_prefix(clang_getCString(typename_cxstr), "enum ");
      clang_disposeString(typename_cxstr);
      proto->set_type_name(typename_str);
      return 0;
    }
    case CXType_ConstantArray: {
      proto->set_label(
          google::protobuf::FieldDescriptorProto_Label_LABEL_REPEATED);
      CXType elem_type = clang_getArrayElementType(field_type);
      auto array_size = clang_getArraySize(field_type);
      proto->mutable_options()
          ->MutableExtension(protostruct::fieldopts)
          ->set_capacity(array_size);
      return set_field_type(proto, elem_type);
    }
    case CXType_Pointer: {
      return 1;
    }
    case CXType_Bool: {
      std::string type_spelling =
          drop_cxstring(clang_getTypeSpelling(field_type));
      auto iter = STDINT_CONVERSIONS.find(type_spelling);
      if (iter != STDINT_CONVERSIONS.end()) {
        type_spelling = iter->second;
      }
      my_options->set_fieldtype(type_spelling);
      proto->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_BOOL);
      return 0;
    }
    default: {
      break;
    }
  }
  LOG(WARNING) << "Unhandled c type case: " << static_cast<int>(field_type.kind)
               << ": " << clang_getTypeKindSpelling(field_type.kind) << "\n";
  return 1;
}

typedef std::tuple<int, int> IntRange;

/// find an existing field by name, or create new one
google::protobuf::FieldDescriptorProto* find_field(
    google::protobuf::DescriptorProto* proto, const std::string& needle) {
  for (int idx = 0; idx < proto->field_size(); idx++) {
    auto* field = proto->mutable_field(idx);
    if (field->name() == needle) {
      return field;
    }
  }

  return nullptr;
}

std::string normalize_type_name(const std::string& type_name) {
  if (type_name.size() == 0) {
    return type_name;
  }
  if (type_name.find(".") == std::string::npos) {
    return type_name;
  }
  return stringutil::split(type_name, '.').back();
}

struct InclusionFilter {
  explicit InclusionFilter(const std::string& pattern, bool exclude = false)
      : pattern{pattern}, regex{pattern}, exclude{exclude} {}

  std::string pattern;
  std::regex regex;

  /// If matching this pattern should *exclude* (versus include) the candidate
  /// string.
  bool exclude;
};

/// Shared state that is needed at all depths of the visitation
class VisitorContext {
 public:
  VisitorContext(CXTranslationUnit tunit,
                 const std::vector<std::string>& source_patterns,
                 const std::string fileset_in)
      : tunit_{tunit} {
    (void)tunit_;

    google::protobuf::FileDescriptorSet file_set{};
    std::ifstream infile{fileset_in};
    if (infile.good()) {
      if (!file_set.ParseFromIstream(&infile)) {
        LOG(FATAL) << "Failed to parse input descriptor set" << fileset_in;
      }
      for (google::protobuf::FileDescriptorProto& filedescr :
           *file_set.mutable_file()) {
        fileproto_map_[filedescr.name()] =
            std::make_shared<google::protobuf::FileDescriptorProto>(filedescr);
      }
    } else {
      LOG(WARNING) << "Input descriptor set " << fileset_in
                   << " does not exist";
    }

    for (const auto& pattern : source_patterns) {
      if (pattern.empty()) {
        continue;
      }
      VLOG(1) << "Compiling regex pattern: " << pattern;
      if (pattern[0] == '!') {
        source_filters_.emplace_back(pattern.substr(1), true);
      } else {
        source_filters_.emplace_back(pattern);
      }
    }
  }

  ~VisitorContext() {}

  bool should_visit(const std::string& filename) {
    auto iter = matched_files_.find(filename);
    if (iter != matched_files_.end()) {
      if (!iter->second) {
        return false;
      }
      return !iter->second->exclude;
    }

    bool visit_me = false;
    const InclusionFilter* matched_filter = nullptr;
    for (const auto& filter : source_filters_) {
      // If the filter matches, then apply the inclusion rule
      if (std::regex_match(filename, filter.regex)) {
        visit_me = !filter.exclude;
        matched_filter = &filter;
      }
    }

    matched_files_[filename] = matched_filter;
    return visit_me;
  }

  /// Called when the FileVisitor chooses visit a cursor
  void note_visit(const std::string& filename) {
    if (source_filepath_ != filename) {
      source_filepath_ = filename;
      file_proto_.reset();
    }
  }

  std::string get_source(CXCursor cursor) {
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation begin = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);
    CXFile cxFile;
    unsigned int beginOff;
    unsigned int endOff;
    clang_getExpansionLocation(begin, &cxFile, 0, 0, &beginOff);
    clang_getExpansionLocation(end, 0, 0, 0, &endOff);

    std::string filename = drop_cxstring(clang_getFileName(cxFile));
    std::string spelling = drop_cxstring(clang_getCursorSpelling(cursor));

    size_t size{0};
    const char* memblock = clang_getFileContents(tunit_, cxFile, &size);
    LOG_IF(FATAL, !memblock) << "Failed to get buffer for file " << filename;

    LOG_IF(FATAL, endOff > size)
        << "File size " << size
        << " is less than the cursor terminal offset of " << endOff
        << " in file " << filename << " for cursor " << spelling;

    return std::string{memblock + beginOff, endOff - beginOff};
  }

  google::protobuf::FileDescriptorProto* file_proto() {
    if (!file_proto_) {
      std::regex extension_re{"\\.[^\\.]+$"};
      std::string protopath =
          std::regex_replace(source_filepath_, extension_re, ".proto");
      auto iter = fileproto_map_.find(protopath);
      if (iter == fileproto_map_.end()) {
        file_proto_ = std::make_shared<google::protobuf::FileDescriptorProto>();
        file_proto_->set_name(protopath);
        fileproto_map_.insert(std::make_pair(protopath, file_proto_));
      } else {
        file_proto_ = iter->second;
      }
      if (file_proto_->mutable_options()
              ->MutableExtension(protostruct::fileopts)
              ->header_filepath()
              .empty()) {
        file_proto_->mutable_options()
            ->MutableExtension(protostruct::fileopts)
            ->set_header_filepath(source_filepath_);
      }
    }
    return file_proto_.get();
  }

  std::map<std::string, std::string>& macros() {
    return macros_;
  }

  bool note_capname(const std::string& capname) {
    /* iter scope */ {
      auto iter = macros_.find(capname);
      if (iter != macros_.end()) {
        file_proto_->mutable_options()
            ->MutableExtension(protostruct::fileopts)
            ->add_capacity_macros(iter->second);
        macros_.erase(iter);
        capname_macros_.insert(capname);
      }
    }

    /* iter scope */ {
      auto iter = capname_macros_.find(capname);
      return iter != capname_macros_.end();
    }
  }

  void log_visits() {
    std::stringstream msg;
    for (auto& pair : matched_files_) {
      if (pair.second) {
        msg << "\n  " << pair.first << " (" << !pair.second->exclude
            << "): " << pair.second->pattern;
      } else {
        msg << "\n  " << pair.first << " (false): ";
      }
    }
    VLOG(1) << "visted files:" << msg.str();
  }

  void get_fileset(google::protobuf::FileDescriptorSet* fileset) {
    fileset->mutable_file()->Reserve(matched_files_.size());
    for (const auto& pair : matched_files_) {
      std::regex extension_re{"\\.[^\\.]+$"};
      std::string protopath =
          std::regex_replace(pair.first, extension_re, ".proto");
      auto iter = fileproto_map_.find(protopath);
      if (iter == fileproto_map_.end()) {
        continue;
      }
      *fileset->add_file() = *iter->second;
    }
  }

 private:
  /// The translation unit built by compiling the header
  CXTranslationUnit tunit_;

  /// The path to the header file we are currently visiting
  std::string source_filepath_;

  /// Pointer to the mutable file descriptor proto that we are building, for the
  /// file that we are currently visiting, and possibly containing information
  /// digested from an "sychronization" .proto
  std::shared_ptr<google::protobuf::FileDescriptorProto> file_proto_;

  /// Macros defined in this file
  std::map<std::string, std::string> macros_;

  /// Macros that were referenced as array lenghts
  std::set<std::string> capname_macros_;

  /// A list of patterns to match against filenames
  std::vector<InclusionFilter> source_filters_;

  /// Map included files to the pattern they matched for inclusion
  std::map<std::string, const InclusionFilter*> matched_files_;

  /// Map from .proto filename to mutable FileDescriptor's for that file
  std::map<std::string, std::shared_ptr<google::protobuf::FileDescriptorProto>>
      fileproto_map_;
};

/// Visit the field declaration for a fixed-length array and look for a
/// numeric constant reference used as the array size.
class FieldVisitor : public ClangVisitor {
 public:
  explicit FieldVisitor(google::protobuf::FieldDescriptorProto* proto,
                        VisitorContext* ctx)
      : proto_{proto}, ctx_{ctx} {}
  virtual ~FieldVisitor() {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    if (clang_getCursorKind(c) == CXCursor_IntegerLiteral) {
      std::string capname = ctx_->get_source(c);
      bool is_macro = ctx_->note_capname(capname);
      if (is_macro) {
        proto_->mutable_options()
            ->MutableExtension(protostruct::fieldopts)
            ->set_capname(capname);
      }
    }
    return CXChildVisit_Recurse;
  }

 private:
  google::protobuf::FieldDescriptorProto* proto_;
  VisitorContext* ctx_;
};

bool fieldname_is_lengthfield(const std::string& fieldname,
                              const std::set<std::string>& array_fieldnames,
                              std::string* associated_fieldname) {
  for (auto candidate_suffix : {"_size", "Count"}) {
    if (stringutil::endswith(fieldname, candidate_suffix)) {
      *associated_fieldname =
          stringutil::pop_suffix(fieldname, candidate_suffix);
      auto found = array_fieldnames.find(*associated_fieldname);
      if (found != array_fieldnames.end()) {
        return true;
      }
    }
  }
  return false;
}

/// Visit each field declaration of a struct and conver it to a field descriptor
/// in the message DescriptorProto `proto`.
class MessageVisitor : public ClangVisitor {
 public:
  explicit MessageVisitor(google::protobuf::DescriptorProto* proto, int msgidx,
                          VisitorContext* ctx)
      : proto_{proto}, msgidx_{msgidx}, ctx_{ctx}, next_number_{1} {
    for (int idx = 0; idx < proto_->reserved_range_size(); idx++) {
      auto range = proto_->reserved_range(idx);
      if (range.has_end()) {
        reserved_ranges_.emplace(range.start(), range.end());
      } else {
        reserved_ranges_.emplace(range.start(),
                                 std::numeric_limits<int>::max());
      }
    }
    for (int idx = 0; idx < proto_->field_size(); idx++) {
      auto& field = proto_->field(idx);
      existing_numbers_.insert(field.number());
    }
  }

  virtual ~MessageVisitor() {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    CXCursorKind kind = clang_getCursorKind(c);
    if (kind != CXCursor_FieldDecl) {
      return CXChildVisit_Continue;
    }

    std::string fieldname = drop_cxstring(clang_getCursorSpelling(c));
    CXType fieldtype = clang_getCursorType(c);

    bool needs_number = false;
    auto* found_field = find_field(proto_, fieldname);
    std::shared_ptr<google::protobuf::FieldDescriptorProto> field;
    if (found_field) {
      if (found_field->options().packed()) {
        LOG(INFO) << "field " << found_field->name() << " is packed";
      }

      field.reset(found_field->New());
      field->CopyFrom(*found_field);

      int err = set_field_type(field.get(), fieldtype);
      if (err) {
        LOG(WARNING) << "Dropping field " << field->name() << " at "
                     << clang_getCursorLocation(c);
        return CXChildVisit_Continue;
      }
      if (found_field->type() != field->type()) {
        LOG(INFO) << "field type for " << field->name() << " changed from "
                  << google::protobuf::FieldDescriptorProto_Type_Name(
                         found_field->type())
                  << " to "
                  << google::protobuf::FieldDescriptorProto_Type_Name(
                         field->type())
                  << " declared at " << clang_getCursorLocation(c)
                  << " for C type: " << clang_getTypeSpelling(fieldtype)
                  << " field decl spelling: " << clang_getCursorSpelling(c);
        needs_number = true;
      }

      if (normalize_type_name(found_field->type_name()) !=
          normalize_type_name(field->type_name())) {
        LOG(INFO) << "type name for " << field->name() << " changed from "
                  << found_field->type_name() << " to " << field->type_name();
        needs_number = true;
      }
    } else {
      field.reset(new google::protobuf::FieldDescriptorProto{});
      field->set_name(fieldname);
      needs_number = true;
      int err = set_field_type(field.get(), fieldtype);
      if (err) {
        LOG(WARNING) << "Dropping field " << field->name() << " at "
                     << clang_getCursorLocation(c);

        return CXChildVisit_Continue;
      }
    }
    if (fieldtype.kind == CXType_ConstantArray) {
      // Recurse the AST and see if the size of the array is a reference to a
      // #define or something
      visit_tree<FieldVisitor>(c, field.get(), ctx_);
    }

    CXString comment_cxstr = clang_Cursor_getRawCommentText(c);
    const char* comment_cstr = clang_getCString(comment_cxstr);
    std::string comment{};
    if (comment_cstr) {
      comment = clang_getCString(comment_cxstr);
      if (comment.find("protostruct: skip") != std::string::npos) {
        LOG(INFO) << "Skipping field " << field->name();
        return CXChildVisit_Continue;
      }
      clang_disposeString(comment_cxstr);
    }

    if (needs_number) {
      field->set_number(get_next_number());
    }

    if (!comment.empty()) {
      field->mutable_options()
          ->MutableExtension(protostruct::fieldopts)
          ->set_comment(comment);

      //  message_type is field #4 within FileDescriptorProto
      int field_idx = output_fields_.size();
      auto* location =
          find_location(ctx_->file_proto(), {4, msgidx_, 2, field_idx});

      // TODO(josh): this is a hack, use clang_Curosr_getCommentRange to
      // determine if it's leading or trailing
      std::string stripped_comment = strip_comment_prefix(comment);
      if (stringutil::startswith(comment, "//!<") ||
          stringutil::startswith(comment, "///<")) {
        location->set_trailing_comments(stripped_comment);
      } else {
        location->set_leading_comments(stripped_comment);
      }
    }

    output_fields_.emplace_back(std::move(field));
    return CXChildVisit_Continue;
  }

 private:
  void _strip_lengthfields() {
    // Set of fieldnames. We search this when trying to determine if a
    // particular fieldname is a count field (according to whether or not
    // it matches the name of another field, suffixed by Count).
    std::set<std::string> fieldnames;

    // Map from field name to the descriptor which mapped it. If we
    // discover a lengthfield then we will use this to look up the associated
    // field and add the lengthfield annotation to it.
    std::map<std::string,
             std::shared_ptr<google::protobuf::FieldDescriptorProto>>
        fieldmap;

    // First, populate our indices.
    for (auto& fieldptr : output_fields_) {
      fieldmap[fieldptr->name()] = fieldptr;
      fieldnames.emplace(fieldptr->name());
    }

    // NOTE(josh): care must be takent to preserve the order of field
    // descriptors so that we can output them in the same order we read them.
    std::vector<std::shared_ptr<google::protobuf::FieldDescriptorProto>>
        output_fields;
    for (auto& fieldptr : output_fields_) {
      std::string associated_fieldname{};
      if (fieldname_is_lengthfield(fieldptr->name(), fieldnames,
                                   &associated_fieldname)) {
        LOG(INFO) << "Matched " << fieldptr->name() << " as a lengthfield for "
                  << associated_fieldname;
        fieldmap[associated_fieldname]
            ->mutable_options()
            ->MutableExtension(protostruct::fieldopts)
            ->set_lenfield(fieldptr->name());
      } else {
        output_fields.emplace_back(fieldptr);
      }
    }
    output_fields_.swap(output_fields);
  }

 public:
  void finish_visit() override {
    _strip_lengthfields();

    proto_->clear_field();
    for (auto& fieldptr : output_fields_) {
      auto* mfield = proto_->add_field();
      mfield->CopyFrom(*fieldptr);
      existing_numbers_.erase(fieldptr->number());
    }
    for (int retired_number : existing_numbers_) {
      auto* range = proto_->add_reserved_range();
      range->set_start(retired_number);
      range->set_end(retired_number + 1);
    }
  }

  int get_next_number() {
    while (true) {
      if (existing_numbers_.find(next_number_) != existing_numbers_.end()) {
        next_number_++;
        continue;
      }

      IntRange query{next_number_, next_number_ + 1};
      auto iter = reserved_ranges_.lower_bound(query);
      if (iter == reserved_ranges_.end()) {
        // Ranges are half open and end is always greater than start, i.e.
        // they are in the form [begin, end). If a range contains the query
        // then it will be pointed to by iter. If iter points to nothing, then
        // the query is not within any ranges.
        break;
      }
      if (next_number_ < std::get<0>(*iter) ||
          std::get<1>(*iter) <= next_number_) {
        // if the query falls within a range, iter points to that range. The
        // start of the range must be before or equal to the query. If the
        // end of the range is before or equal to the query, then the query
        // is not contained (ranges are half open);
        break;
      }
      next_number_++;
    }
    return next_number_++;
  }

 private:
  google::protobuf::DescriptorProto* proto_;
  int msgidx_;
  VisitorContext* ctx_;
  int next_number_;
  std::set<IntRange> reserved_ranges_;
  std::set<int> existing_numbers_;
  std::vector<std::shared_ptr<google::protobuf::FieldDescriptorProto>>
      output_fields_;
};

/// If a message named `needle` already exists in the FileDesciptorProto
/// `proto`, then return a mutable pointer to it's message DescriptorProto.
/// Otherwise, create a new message DesciptorProto for named `needle` and then
/// return a mutable pointer to the newly created descriptor.
google::protobuf::DescriptorProto* find_message(
    google::protobuf::FileDescriptorProto* proto, std::string needle,
    int* found_idx) {
  std::string needle_stripped;
  if (needle.substr(0, 2) == "dw") {
    needle_stripped = needle.substr(2);
  } else {
    needle_stripped = needle;
  }

  for (int idx = 0; idx < proto->message_type_size(); idx++) {
    google::protobuf::DescriptorProto* candidate_proto =
        proto->mutable_message_type(idx);
    std::string candidate_name = candidate_proto->name();
    std::string candidate_name_stripped;
    if (candidate_name.substr(0, 2) == "dw") {
      candidate_name_stripped = candidate_name.substr(2);
    } else {
      candidate_name_stripped = candidate_name;
    }

    if (stringutil::to_lower(candidate_name_stripped) ==
        stringutil::to_lower(needle_stripped)) {
      if (candidate_name != needle) {
        LOG(WARNING) << "message " << candidate_name << " changed to "
                     << needle;
        candidate_proto->set_name(needle);
      }
      *found_idx = idx;
      return candidate_proto;
    }
  }

  *found_idx = proto->message_type_size();
  auto* new_msg = proto->add_message_type();
  new_msg->set_name(needle);

  return new_msg;
}

/// Top level visitor: Recursively walk the AST looking for enum or struct
/// definitions and dispatch specific visitors for each.
class FileVisitor : public ClangVisitor {
 public:
  explicit FileVisitor(VisitorContext* ctx) : ctx_{ctx} {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    CXSourceLocation cursor_location = clang_getCursorLocation(c);
    CXFile cursor_file{};
    unsigned int cursor_line{0};
    unsigned int cursor_column{0};
    clang_getSpellingLocation(cursor_location, &cursor_file, &cursor_line,
                              &cursor_column, nullptr);

    std::string filename = drop_cxstring(clang_getFileName(cursor_file));
    if (filename.substr(0, 2) == "./") {
      filename = filename.substr(2);
    }
    if (!ctx_->should_visit(filename)) {
      return CXChildVisit_Continue;
    }
    ctx_->note_visit(filename);

    std::stringstream strm{};
    strm << clang_getCursorSpelling(c);
    std::string cursor_spelling = strm.str();
    CXCursorKind kind = clang_getCursorKind(c);

    switch (kind) {
      case CXCursor_InclusionDirective: {
        CXFile included_file = clang_getIncludedFile(c);
        std::string included_filename =
            drop_cxstring(clang_getFileName(included_file));
        ctx_->file_proto()
            ->mutable_options()
            ->MutableExtension(protostruct::fileopts)
            ->add_included_files(included_filename);
        break;
      }

      case CXCursor_EnumDecl: {
        auto pair = visited_enums_.insert(cursor_spelling);
        if (!pair.second) {
          // This is the second time we've hit this decl node. This happens
          // because typedefed structs get enumerated twice.
          return CXChildVisit_Continue;
        }

        std::string comment = get_cursor_comment(c);
        if (comment.find("protostruct: skip") != std::string::npos) {
          LOG(INFO) << "Skipping enum " << clang_getCursorSpelling(c);
          return CXChildVisit_Continue;
        }

        std::string needle{};
        /* temp scope */ {
          CXString namestr = clang_getCursorSpelling(c);
          needle = clang_getCString(namestr);
          clang_disposeString(namestr);
        }
        int enumidx = 0;
        auto* enum_proto = find_enum(ctx_->file_proto(), needle, &enumidx);
        visit_tree<EnumVisitor>(c, enum_proto, enumidx, ctx_->file_proto());

        if (comment.size() > 0) {
          enum_proto->mutable_options()
              ->MutableExtension(protostruct::enumopts)
              ->set_comment(comment);

          //  enum_type is field #5 within FileDescriptorProto
          auto* location = find_location(ctx_->file_proto(), {5, enumidx});
          std::string stripped_comment = strip_comment_prefix(comment);
          location->set_leading_comments(stripped_comment);
        }

        return CXChildVisit_Continue;
      }

      case CXCursor_StructDecl: {
        auto pair = visited_enums_.insert(cursor_spelling);
        if (!pair.second) {
          // This is the second time we've hit this decl node. This happens
          // because typedefed structs get enumerated twice.
          return CXChildVisit_Continue;
        }

        std::string comment = get_cursor_comment(c);
        if (comment.find("protostruct: skip") != std::string::npos) {
          LOG(INFO) << "Skipping struct " << clang_getCursorSpelling(c);
          return CXChildVisit_Continue;
        }

        std::string needle{};
        /* temp scope */ {
          CXString namestr = clang_getCursorSpelling(c);
          needle = clang_getCString(namestr);
          clang_disposeString(namestr);
        }

        int msgidx = 0;
        auto* msg_proto = find_message(ctx_->file_proto(), needle, &msgidx);
        visit_tree<MessageVisitor>(c, msg_proto, msgidx, ctx_);

        if (comment.size() > 0) {
          msg_proto->mutable_options()
              ->MutableExtension(protostruct::msgopts)
              ->set_comment(comment);

          //  message_Type is field #4 within FileDescriptorProto
          auto* location = find_location(ctx_->file_proto(), {4, msgidx});
          std::string stripped_comment = strip_comment_prefix(comment);
          location->set_leading_comments(stripped_comment);
        }
        return CXChildVisit_Continue;
      }

      case CXCursor_MacroDefinition: {
        // NOTE(josh): clang_getCursorSpelling yields only the name of the
        // macro, not it's definition/expansions

        std::stringstream strm{};
        strm << clang_getCursorSpelling(c);
        std::string name = strm.str();
        std::string defn = ctx_->get_source(c);
        ctx_->macros()[name] = defn;
        break;
      }

      default:
        break;
    }

    return CXChildVisit_Recurse;
  }

  void finish_visit() override {
    ctx_->log_visits();
  }

 private:
  std::set<std::string> visited_enums_;
  std::set<std::string> visited_messages_;
  VisitorContext* ctx_;
};

// NOTE(josh): we don't want to use limits.h because we aren't going to be
// running this on the system we compile for. Also PATH_MAX is a lie and the
// underlying filesystem probably supports larger paths.
constexpr size_t _path_max = 4096;

std::wstring widen(const std::string& narrow_bytes) {
  std::wstring wide_bytes{};
  wide_bytes.resize(narrow_bytes.size());

  size_t result = mbstowcs(&wide_bytes[0], &narrow_bytes[0], wide_bytes.size());
  wide_bytes.resize(result);

  return wide_bytes;
}

std::wstring get_path_to_executable() {
  std::array<char, _path_max> path_buf;
  ssize_t result = readlink("/proc/self/exe", &path_buf[0], _path_max);

  if (result < 0 || static_cast<size_t>(result) >= _path_max - 1) {
    return std::wstring();
  }

  // readlink does not append a terminating null
  path_buf[result] = '\0';

  return widen(&path_buf[0]);
}

int python_gen(const std::vector<std::string>& args) {
  // Insert the path to this file as argv[1] so that python will execute it.
  // This file is expected to be a concatenation of this program (ELF file) one
  // or more binary blobs and then zipfile containing python source code.
  std::wstring my_exe = get_path_to_executable();

  std::vector<std::wstring> my_args = {L"protostruct",
                                       get_path_to_executable()};
  for (const auto& arg : args) {
    my_args.emplace_back(widen(arg));
  }
  int my_argc = my_args.size();

  // NOTE(josh): must include an additional null entry to terminate the array
  std::vector<wchar_t*> my_argv(my_argc + 1, nullptr);
  for (int idx = 0; idx < my_argc; idx++) {
    my_argv[idx] = &(my_args[idx][0]);
  }

  Py_SetProgramName(my_argv[0]);
  Py_Initialize();
  int err = Py_Main(my_argc, &my_argv[0]);
  Py_Finalize();
  return err;
}

int compile_main(const ProgramOptions& popts) {
  auto& copts = popts.compile_opts;
  if (copts.binary_outpath.empty()) {
    LOG(FATAL) << "--pb3-out is required";
  }

  std::string descriptor_set_inpath = popts.descriptor_set_inpath;
  if (descriptor_set_inpath.empty()) {
    // If a synchronization proto was not provided, default to the output
    // proto. That way, if the output proto already exists, we will read
    // it in and preserve any choices that were made.
    descriptor_set_inpath = copts.binary_outpath;
  }

  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit tunit{};

  std::vector<const char*> clang_argv;
  clang_argv.reserve(copts.clang_options.size() + 10);
  for (const auto& arg : copts.clang_options) {
    clang_argv.push_back(&arg[0]);
  }

  // TODO(josh): don't bake these here.
  // NOTE(josh): previously used -std=c++11 and -language=c++ but would fail
  // to parse test_messages.h on clang-13
  clang_argv.push_back("-std=c99");
  clang_argv.push_back("-language=c");

#if CINDEX_VERSION == CINDEX_VERSION_ENCODE(0, 50)
  // NOTE(josh): clang8 can't seem to find standard headers
  clang_argv.push_back("-isystem");
  clang_argv.push_back("/usr/lib/llvm-8/lib/clang/8.0.0/include");
  clang_argv.push_back("-isystem");
  clang_argv.push_back("/usr/lib/llvm-8/lib/clang/8.0.1/include");
#endif

  CXErrorCode code = clang_parseTranslationUnit2(
      index, copts.source_filepath.c_str(), &clang_argv[0], clang_argv.size(),
      nullptr, 0, CXTranslationUnit_DetailedPreprocessingRecord, &tunit);

  if (code != CXError_Success) {
    LOG(ERROR) << "Failed to build translation unit for "
               << copts.source_filepath << " code: " << code
               << " argv:" << stringutil::join(clang_argv, " ") << "\n";
  }

  if (clang_getNumDiagnostics(tunit)) {
    LOG(WARNING) << "CINDEX_VERSION" << CINDEX_VERSION_MAJOR << "."
                 << CINDEX_VERSION_MINOR;
  }
  for (size_t idx = 0; idx < clang_getNumDiagnostics(tunit); idx++) {
    CXDiagnostic diag = clang_getDiagnostic(tunit, idx);
    LOG(WARNING) << clang_getDiagnosticSpelling(diag);
    clang_disposeDiagnostic(diag);
  }

  if (code != CXError_Success) {
    exit(1);
  }

  auto source_patterns = copts.source_patterns;
  if (source_patterns.empty()) {
    source_patterns.emplace_back(copts.source_filepath);
  }

  VisitorContext ctx{tunit, source_patterns, descriptor_set_inpath};
  visit_tree<FileVisitor>(clang_getTranslationUnitCursor(tunit), &ctx);
  google::protobuf::FileDescriptorSet fileset{};

  ctx.get_fileset(&fileset);
  std::ofstream outfile{copts.binary_outpath};
  if (!outfile.good()) {
    LOG(FATAL) << "Failed to open " << copts.binary_outpath << " for write";
  }
  fileset.SerializeToOstream(&outfile);
  outfile.close();

  clang_disposeTranslationUnit(tunit);
  clang_disposeIndex(index);

  // NOTE(josh): See below as an example of how to get a FileDescriptor from
  // the FileDescriptorProto and how to access comments.
  // google::protobuf::DescriptorPool dpool{};
  // const google::protobuf::FileDescriptor* proto_file =
  //     dpool.BuildFile(fileproto);
  // if (!proto_file) {
  //   exit(1);
  // }

  // for (int idx = 0; idx < proto_file->message_type_count(); idx++) {
  //   google::protobuf::SourceLocation loc{};
  //   proto_file->message_type(idx)->GetSourceLocation(&loc);

  //   LOG(WARNING) << loc.leading_comments << "\n";
  //   LOG(WARNING) << loc.trailing_comments << "\n";
  //   for (auto comment : loc.leading_detached_comments) {
  //     LOG(WARNING) << comment << "\n";
  //   }

  //   for (int jdx = 0; jdx < proto_file->message_type(idx)->field_count();
  //        jdx++) {
  //     proto_file->message_type(idx)->field(jdx)->GetSourceLocation(&loc);
  //     LOG(WARNING) << loc.leading_comments << "\n";
  //     LOG(WARNING) << loc.trailing_comments << "\n";
  //     for (auto comment : loc.leading_detached_comments) {
  //       LOG(WARNING) << comment << "\n";
  //     }
  //   }
  // }

  return 0;
}

void compile_descriptor_set(const ProgramOptions& popts) {
  google::protobuf::compiler::DiskSourceTree source_tree{};
  for (auto& proto_path : popts.proto_path) {
    source_tree.MapPath("", proto_path);
  }

  MyErrorCollector error_collector{};
  google::protobuf::compiler::SourceTreeDescriptorDatabase descr_db{
      &source_tree};
  descr_db.RecordErrorsTo(&error_collector);

  google::protobuf::FileDescriptorSet fileset{};
  for (const auto& proto_inpath : popts.proto_inpaths) {
    bool success = descr_db.FindFileByName(proto_inpath, fileset.add_file());
    TANGENT_ASSERT(success) << "Failed to import " << proto_inpath;
  }

  std::ofstream outfile{popts.descriptor_set_inpath};
  TANGENT_ASSERT(outfile.good())
      << "Failed to open " << popts.descriptor_set_inpath << " for write";
  fileset.SerializeToOstream(&outfile);
  outfile.flush();
}

int gen_main(const ProgramOptions& popts) {
  auto& gopts = popts.gen_opts;
  std::vector<std::string> args{};

  if (!gopts.cpp_root.empty()) {
    args.push_back("--cpp-root");
    args.push_back(gopts.cpp_root);
  }
  args.push_back(popts.descriptor_set_inpath);
  args.insert(args.end(), gopts.templates.begin(), gopts.templates.end());
  return python_gen(args);
}
