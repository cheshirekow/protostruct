// Copyright (C) 2021 Josh Bialkowski (josh.bialkowski@gmail.com)
#include "tangent/tjson/cpputil.h"
#include "tangent/util/exception.h"

std::ostream& operator<<(std::ostream& out,
                         const tjson_SourceLocation& location) {
  out << location.lineno << ":" << location.colno;
  return out;
}

std::ostream& operator<<(std::ostream& out, const tjson_Error& error) {
  out << "[" << error.code << "](" << error.loc << "): " << error.msg;
  return out;
}

std::ostream& operator<<(std::ostream& out, const tjson_StringPiece& str) {
  for (const char* cptr = str.begin; cptr < str.end; cptr++) {
    out << *cptr;
  }
  return out;
}

namespace tjson {

int parse(tjson_ParseContext ctx, uint64_t* value) {
  return tjson_parse_uint64(ctx, value);
}

int parse(tjson_ParseContext ctx, uint32_t* value) {
  return tjson_parse_uint32(ctx, value);
}

int parse(tjson_ParseContext ctx, uint16_t* value) {
  return tjson_parse_uint16(ctx, value);
}

int parse(tjson_ParseContext ctx, uint8_t* value) {
  return tjson_parse_uint8(ctx, value);
}

int parse(tjson_ParseContext ctx, int64_t* value) {
  return tjson_parse_int64(ctx, value);
}

int parse(tjson_ParseContext ctx, int32_t* value) {
  return tjson_parse_int32(ctx, value);
}

int parse(tjson_ParseContext ctx, int16_t* value) {
  return tjson_parse_int16(ctx, value);
}

int parse(tjson_ParseContext ctx, int8_t* value) {
  return tjson_parse_int8(ctx, value);
}

int parse(tjson_ParseContext ctx, double* value) {
  return tjson_parse_double(ctx, value);
}

int parse(tjson_ParseContext ctx, float* value) {
  return tjson_parse_float(ctx, value);
}

int parse(tjson_ParseContext ctx, bool* value) {
  int8_t ivalue{0};
  int err = tjson_parse_boolean(ctx, &ivalue);
  *value = static_cast<bool>(ivalue);
  return err;
}

int parse(tjson_ParseContext ctx, std::string* str) {
  struct tjson_Event event {};
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }

  // Strip quotes
  tjson_StringPiece value =
      tjson_StringPiece_substr(event.token.spelling, 1, -1);

  // TODO(josh): need to decode
  str->clear();
  str->insert(0, value.begin, (value.end - value.begin));
  return 0;
}

}  // namespace tjson
