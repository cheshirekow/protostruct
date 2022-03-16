#pragma once
// Copyright (C) 2021 Josh Bialkowski (josh.bialkowski@gmail.com)
#include "tangent/tjson/parse.h"
#include "tangent/tjson/tjson.h"
#include "tangent/util/exception.h"
#include "tangent/util/type_string.h"

#include <string>
#include <vector>

namespace tjson {

int parse(tjson_ParseContext ctx, uint64_t* value);
int parse(tjson_ParseContext ctx, uint32_t* value);
int parse(tjson_ParseContext ctx, uint16_t* value);
int parse(tjson_ParseContext ctx, uint8_t* value);
int parse(tjson_ParseContext ctx, int64_t* value);
int parse(tjson_ParseContext ctx, int32_t* value);
int parse(tjson_ParseContext ctx, int16_t* value);
int parse(tjson_ParseContext ctx, int8_t* value);
int parse(tjson_ParseContext ctx, double* value);
int parse(tjson_ParseContext ctx, float* value);

int parse(tjson_ParseContext ctx, bool* value);
int parse(tjson_ParseContext ctx, std::string* value);

template <class T>
void parse_json(const std::string& json_str, T* value) {
  tjson_Error error{};
  tjson_LexerParser stream{};
  TANGENT_ASSERT(!tjson_LexerParser_init(&stream, &error))
      << "Failed to initialized tjson stream: " << error.msg;

  if (tjson_LexerParser_begin(
          &stream, tjson_StringPiece_fromstr(json_str.c_str()), &error)) {
    std::stringstream message;
    message << "Failed to start tjson parser: " << error.msg;
    throw std::runtime_error(message.str());
  }

  tjson_ParseContext ctx{};
  ctx.stream = &stream;
  ctx.error = &error;

  TANGENT_ASSERT(!parse(ctx, value))
      << "Failed to parse " << type_string<T>() << error << "\n"
      << json_str;
}

template <class T, class Allocator>
int vector_listitem_callback(std::vector<T, Allocator>* value,
                             tjson_ParseContext ctx) {
  value->emplace_back();
  return parse(ctx, &value->back());
}

template <class T, class Allocator>
int parse(tjson_ParseContext ctx, std::vector<T, Allocator>* value) {
  return tjson_parse_list(ctx,
                          reinterpret_cast<tjson_listitem_callback>(
                              &vector_listitem_callback<T, Allocator>),
                          value);
}

}  // namespace tjson

std::ostream& operator<<(std::ostream& out,
                         const tjson_SourceLocation& location);
std::ostream& operator<<(std::ostream& out, const tjson_Error& error);
std::ostream& operator<<(std::ostream& out, const tjson_StringPiece& str);
