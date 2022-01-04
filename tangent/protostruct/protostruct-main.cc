// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <sys/types.h>
#include <unistd.h>

#include <Python.h>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <clang-c/Index.h>
#include <glog/logging.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/wire_format_lite.h>

#include "argue/argue.h"
#include "tangent/protostruct/descriptor_extensions.pb.h"
#include "tangent/util/fallthrough.h"
#include "tangent/util/stringutil.h"

#define TANGENT_PROTOSTRUCT_VERSION \
  { 0, 1, 1, "dev", 0 }

// NOTE(josh): to self, see:
// https://developers.google.com/protocol-buffers/docs/reference/cpp#google.protobuf.compiler
// for c++ API to read .proto.

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

struct ProgramOptions {
  std::string source_filepath;
  std::string proto_filepath;

  std::string proto_out;
  std::string cpp_out;

  std::string output_filepath{"-"};
  std::string step{"both"};
  std::vector<std::string> proto_path;
  std::vector<std::string> clang_options;
  std::vector<std::string> only_list;
};

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

void setup_parser(argue::Parser* parser, ProgramOptions* opts) {
  using argue::keywords::action;
  using argue::keywords::choices;
  using argue::keywords::default_;
  using argue::keywords::dest;
  using argue::keywords::help;
  using argue::keywords::nargs;

  // clang-format off
  parser->add_argument(
      "sourcefile", dest=&(opts->source_filepath), nargs='?',
      help="The source file to parse");

  parser->add_argument(
      "-s", "--step", dest=&(opts->step), default_="both",
      choices={"compile", "gen", "both"},
      help="There are two steps `compile` and `gen`. The default is `both`");

  parser->add_argument(
      "-o", "--outfile", dest=&(opts->output_filepath),
      help="The output file where a binary encoded FileDescriptorProto is"
           " written. This will be processed by protostruct-gen to"
           " update the .proto file. The default `-` is stdout, and will"
           " be piped to protostruct-gen");

  parser->add_argument(
      "-p", "--proto-in", dest=&(opts->proto_filepath),
      help="Override the proto file to synchronize. The default path is "
           "constructed based on --proto-out but use this force a different "
           "input file.");

  parser->add_argument(
      "--proto-path", action="append", dest=&(opts->proto_path),
      help="Root directory of the search tree for dependant protocol buffer."
           " definitions. Can be specified multiple times.");

  parser->add_argument(
      "--proto-out", dest=&(opts->proto_out),
      help="Root directory of the output tree for generated .proto files");

  parser->add_argument(
      "--cpp-out", dest=&(opts->cpp_out),
      help="Root directory of the output tree for generated C and C++ files");

  parser->add_argument(
      "--only", dest=&(opts->only_list), nargs="+",
      help="Only generate bindings of these types");

  parser->add_argument(
      "remainder", nargs=argue::REMAINDER, dest=&(opts->clang_options),
      help="Command line flags passed to clang");

  // clang-format on
}

class ClangVisitor {
 public:
  virtual CXChildVisitResult visit(CXCursor c, CXCursor parent) = 0;

  virtual void finish_visit() {}

  static CXChildVisitResult Callback(CXCursor c, CXCursor parent,
                                     CXClientData client_data) {
    return reinterpret_cast<ClangVisitor*>(client_data)->visit(c, parent);
  }
};

template <class VisitorType, class... Args>
void visit_tree(CXCursor cursor, Args&&... args) {
  VisitorType visitor{std::forward<Args>(args)...};
  clang_visitChildren(cursor, ClangVisitor::Callback, &visitor);
  visitor.finish_visit();
}

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
          ->MutableExtension(google::protobuf::ProtostructEnumValueOptions::
                                 protostruct_options)
          ->set_protostruct_comment(comment);

      //  enum_type is field #5 within FileDescriptorProto
      auto* location = find_location(file_proto_, {5, enumidx_, 2, value_idx});
      std::string stripped_comment = strip_comment_prefix(comment);

      // TODO(josh): this is a hack, use clang_Curosr_getCommentRange to
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

google::protobuf::EnumDescriptorProto* find_enum(
    google::protobuf::FileDescriptorProto* proto, std::string needle,
    int* found_idx) {
  std::string needle_stripped;
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

    case CXType_UInt:
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_INT32);
      out->push_back(google::protobuf::FieldDescriptorProto_Type_TYPE_UINT32);
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

int set_field_type(google::protobuf::FieldDescriptorProto* proto,
                   CXType field_type) {
  auto* my_options = proto->mutable_options()->MutableExtension(
      google::protobuf::ProtostructFieldOptions::protostruct_options);
  if (!my_options->has_protostruct_type()) {
    CXString cxstr = clang_getTypeSpelling(field_type);
    const char* cstr = clang_getCString(cxstr);
    my_options->set_protostruct_type(cstr);
    clang_disposeString(cxstr);
  }

  switch (field_type.kind) {
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
          ->MutableExtension(
              google::protobuf::ProtostructFieldOptions::protostruct_options)
          ->set_array_size(array_size);
      return set_field_type(proto, elem_type);
    }
    case CXType_Pointer: {
      return 1;
    }
    case CXType_Bool: {
      proto->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_BOOL);
      return 0;
    }
    default: {
      break;
    }
  }
  LOG(WARNING) << "Unhandled c type case: "
               << clang_getTypeKindSpelling(field_type.kind) << "\n";
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

class FieldVisitor : public ClangVisitor {
 public:
  explicit FieldVisitor(google::protobuf::FieldDescriptorProto* proto)
      : proto_{proto} {
    (void)proto_;
  }
  virtual ~FieldVisitor() {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    LOG(INFO) << "[" << clang_getCursorKindSpelling(clang_getCursorKind(c))
              << "]: " << clang_getCursorSpelling(c);
    return CXChildVisit_Recurse;
  }

 private:
  google::protobuf::FieldDescriptorProto* proto_;
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

class MessageVisitor : public ClangVisitor {
 public:
  explicit MessageVisitor(google::protobuf::DescriptorProto* proto, int msgidx,
                          google::protobuf::FileDescriptorProto* file_proto)
      : proto_{proto},
        msgidx_{msgidx},
        file_proto_{file_proto},
        next_number_{1} {
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

    std::string fieldname;
    /* temp scope */ {
      CXString namestr = clang_getCursorSpelling(c);
      fieldname = clang_getCString(namestr);
      clang_disposeString(namestr);
    }

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
    // visit_tree<FieldVisitor>(c, field.get());

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
          ->MutableExtension(
              google::protobuf::ProtostructFieldOptions::protostruct_options)
          ->set_protostruct_comment(comment);

      //  message_type is field #4 within FileDescriptorProto
      int field_idx = output_fields_.size();
      auto* location = find_location(file_proto_, {4, msgidx_, 2, field_idx});

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
    std::set<std::string> fieldnames;
    std::map<std::string,
             std::shared_ptr<google::protobuf::FieldDescriptorProto>>
        fieldmap;

    for (auto& fieldptr : output_fields_) {
      fieldmap[fieldptr->name()] = fieldptr;
      fieldnames.emplace(fieldptr->name());
    }

    for (auto& fieldname : fieldnames) {
      std::string associated_fieldname{};
      if (fieldname_is_lengthfield(fieldname, fieldnames,
                                   &associated_fieldname)) {
        LOG(INFO) << "Matched " << fieldname << " as a lengthfield for "
                  << associated_fieldname;
        fieldmap.erase(fieldname);
        fieldmap[associated_fieldname]
            ->mutable_options()
            ->MutableExtension(
                google::protobuf::ProtostructFieldOptions::protostruct_options)
            ->set_length_field(fieldname);
      }
    }

    output_fields_.clear();
    for (auto& pair : fieldmap) {
      output_fields_.emplace_back(pair.second);
    }
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
  google::protobuf::FileDescriptorProto* file_proto_;
  int next_number_;
  std::set<IntRange> reserved_ranges_;
  std::set<int> existing_numbers_;
  std::vector<std::shared_ptr<google::protobuf::FieldDescriptorProto>>
      output_fields_;
};

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

class FileVisitor : public ClangVisitor {
 public:
  FileVisitor(CXFile file_of_interest,
              google::protobuf::FileDescriptorProto* proto)
      : file_of_interest_{file_of_interest}, file_proto_{proto} {}

  CXChildVisitResult visit(CXCursor c, CXCursor parent) override {
    CXSourceLocation cursor_location = clang_getCursorLocation(c);
    CXFile cursor_file{};
    unsigned int cursor_line{0};
    unsigned int cursor_column{0};
    clang_getSpellingLocation(cursor_location, &cursor_file, &cursor_line,
                              &cursor_column, nullptr);
    if (!clang_File_isEqual(file_of_interest_, cursor_file)) {
      return CXChildVisit_Continue;
    }

    std::stringstream strm{};
    strm << clang_getCursorSpelling(c);
    std::string cursor_spelling = strm.str();
    CXCursorKind kind = clang_getCursorKind(c);

    switch (kind) {
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
        auto* enum_proto = find_enum(file_proto_, needle, &enumidx);
        visit_tree<EnumVisitor>(c, enum_proto, enumidx, file_proto_);

        if (comment.size() > 0) {
          enum_proto->mutable_options()
              ->MutableExtension(
                  google::protobuf::ProtostructEnumOptions::protostruct_options)
              ->set_protostruct_comment(comment);

          //  enum_type is field #5 within FileDescriptorProto
          auto* location = find_location(file_proto_, {5, enumidx});
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
        auto* msg_proto = find_message(file_proto_, needle, &msgidx);
        visit_tree<MessageVisitor>(c, msg_proto, msgidx, file_proto_);

        if (comment.size() > 0) {
          msg_proto->mutable_options()
              ->MutableExtension(google::protobuf::ProtostructMessageOptions::
                                     protostruct_options)
              ->set_protostruct_comment(comment);

          //  message_Type is field #4 within FileDescriptorProto
          auto* location = find_location(file_proto_, {4, msgidx});
          std::string stripped_comment = strip_comment_prefix(comment);
          location->set_leading_comments(stripped_comment);
        }
        return CXChildVisit_Continue;
      }

      default:
        break;
    }

    return CXChildVisit_Recurse;
  }

 private:
  std::set<std::string> visited_enums_;
  std::set<std::string> visited_messages_;
  CXFile file_of_interest_;
  google::protobuf::FileDescriptorProto* file_proto_;
};

void process_file(CXTranslationUnit tunit, CXFile file_of_interest,
                  google::protobuf::FileDescriptorProto* proto) {
  visit_tree<FileVisitor>(clang_getTranslationUnitCursor(tunit),
                          file_of_interest, proto);
}

int compile_main(const ProgramOptions& popts) {
  google::protobuf::compiler::DiskSourceTree source_tree{};
  for (auto& proto_path : popts.proto_path) {
    source_tree.MapPath("", proto_path);
  }

  std::string proto_filepath = popts.proto_filepath;
  if (proto_filepath.empty()) {
    auto name_parts = stringutil::split(popts.source_filepath, '.');
    name_parts.pop_back();
    name_parts.push_back("proto");
    proto_filepath = stringutil::join(name_parts, ".");
  }

  MyErrorCollector error_collector{};
  google::protobuf::compiler::SourceTreeDescriptorDatabase descr_db{
      &source_tree};
  descr_db.RecordErrorsTo(&error_collector);

  google::protobuf::FileDescriptorProto fileproto{};
  if (!descr_db.FindFileByName(proto_filepath, &fileproto)) {
    LOG(FATAL) << "Failed to import " << proto_filepath;
    exit(1);
  }

  fileproto.mutable_options()
      ->MutableExtension(
          google::protobuf::ProtostructFileOptions::protostruct_options)
      ->set_header_filepath(popts.source_filepath);

  if (!stringutil::endswith(popts.source_filepath, ".proto")) {
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit tunit{};

    std::vector<const char*> clang_argv;
    clang_argv.reserve(popts.clang_options.size() + 10);
    for (const auto& arg : popts.clang_options) {
      clang_argv.push_back(&arg[0]);
    }

    // TODO(josh): don't bake these here.
    // NOTE(josh): previously used -std=c++11 and -language=c++ but would fail
    // to parse test_messages.h on clang-13
    clang_argv.push_back("-std=c99");
    clang_argv.push_back("-language=c");

    CXErrorCode code = clang_parseTranslationUnit2(
        index, popts.source_filepath.c_str(), &clang_argv[0], clang_argv.size(),
        nullptr, 0, CXTranslationUnit_None, &tunit);
    if (code != CXError_Success) {
      std::cerr << "Failed to build translation unit for "
                << popts.source_filepath << " code: " << code << " argv:";

      for (const char* arg : clang_argv) {
        std::cerr << " " << arg;
      }
      std::cerr << "\n";

      for (size_t idx = 0; idx < clang_getNumDiagnostics(tunit); idx++) {
        CXDiagnostic diag = clang_getDiagnostic(tunit, idx);
        std::cerr << clang_getDiagnosticSpelling(diag);
        clang_disposeDiagnostic(diag);
      }

      exit(1);
    }

    CXFile file_of_interest =
        clang_getFile(tunit, popts.source_filepath.c_str());
    process_file(tunit, file_of_interest, &fileproto);

    clang_disposeTranslationUnit(tunit);
    clang_disposeIndex(index);
  }

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

  std::ofstream outfile{popts.output_filepath};
  if (!outfile.good()) {
    LOG(FATAL) << "Failed to open " << popts.output_filepath << " for write";
  }

  fileproto.SerializeToOstream(&outfile);
  return 0;
}

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

int gen_main(const ProgramOptions& popts) {
  // Insert the path to this file as argv[1] so that python will execute it.
  // This file is expected to be a concatenation of this program (ELF file) one
  // or more binary blobs and then zipfile containing python source code.
  std::wstring my_exe = get_path_to_executable();

  std::vector<std::wstring> my_args = {L"protostruct",
                                       get_path_to_executable(),
                                       widen(popts.output_filepath),  // infile
                                       L"--proto-out",
                                       widen(popts.proto_out),
                                       L"--cpp-out",
                                       widen(popts.cpp_out)};
  if (!popts.only_list.empty()) {
    my_args.emplace_back(L"--only");
    for (auto only : popts.only_list) {
      my_args.emplace_back(widen(only));
    }
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
  setup_parser(&argparser, &popts);

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

  int err = 0;

  if (popts.step == "compile" || popts.step == "both") {
    err = compile_main(popts);
  }
  if (err != 0) {
    return err;
  }

  if (popts.step == "gen" || popts.step == "both") {
    err = gen_main(popts);
  }
  if (err != 0) {
    return err;
  }

  return 0;
}
