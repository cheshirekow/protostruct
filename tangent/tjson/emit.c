// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/tjson/emit.h"

#include <stdio.h>
#include <stdlib.h>

static const char* get_unsigned_format(size_t size) {
  static int8_t is_initialized = 0;
  static const char* formats[9] = {"%u", "%u", "%u", "%u", "%u",
                                   "%u", "%u", "%u", "%u"};
  if (!is_initialized) {
    formats[sizeof(long long unsigned int)] = "%llu";
    formats[sizeof(long unsigned int)] = "%lu";
    formats[sizeof(unsigned int)] = "%u";
    formats[sizeof(unsigned short)] = "%hu";
    formats[sizeof(unsigned char)] = "%hhu";
    is_initialized = 1;
  }
  return formats[size];
}

static const char* get_signed_format(size_t size) {
  static int8_t is_initialized = 0;
  static const char* formats[9] = {"%i", "%i", "%i", "%i", "%i",
                                   "%i", "%i", "%i", "%i"};
  if (!is_initialized) {
    formats[sizeof(long long int)] = "%lli";
    formats[sizeof(long int)] = "%li";
    formats[sizeof(int)] = "%i";
    formats[sizeof(short)] = "%hi";
    formats[sizeof(char)] = "%hhi";
    is_initialized = 1;
  }
  return formats[size];
}

void tjson_emit_uint64(struct tjson_WriteBuffer* buf, uint64_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_unsigned_format(sizeof(uint64_t)), value);
  buf->begin += count;
}

void tjson_emit_uint32(struct tjson_WriteBuffer* buf, uint32_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_unsigned_format(sizeof(uint32_t)), value);
  buf->begin += count;
}

void tjson_emit_uint16(struct tjson_WriteBuffer* buf, uint16_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_unsigned_format(sizeof(uint16_t)), value);
  buf->begin += count;
}

void tjson_emit_uint8(struct tjson_WriteBuffer* buf, uint8_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_unsigned_format(sizeof(uint16_t)), (uint16_t)value);
  buf->begin += count;
}

void tjson_emit_int64(struct tjson_WriteBuffer* buf, int64_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_signed_format(sizeof(int64_t)), value);
  buf->begin += count;
}

void tjson_emit_int32(struct tjson_WriteBuffer* buf, int32_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_signed_format(sizeof(int32_t)), value);
  buf->begin += count;
}

void tjson_emit_int16(struct tjson_WriteBuffer* buf, int16_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_signed_format(sizeof(int16_t)), value);
  buf->begin += count;
}

void tjson_emit_int8(struct tjson_WriteBuffer* buf, int8_t value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin),
                       get_signed_format(sizeof(int16_t)), (int16_t)value);
  buf->begin += count;
}

void tjson_emit_double(struct tjson_WriteBuffer* buf, double value) {
  int count = snprintf(buf->begin, (buf->end - buf->begin), "%e", value);
  buf->begin += count;
}

void tjson_emit_float(struct tjson_WriteBuffer* buf, double value) {
  int count =
      snprintf(buf->begin, (buf->end - buf->begin), "%e", (double)value);
  buf->begin += count;
}

void tjson_emit_boolean(struct tjson_WriteBuffer* buf, int8_t value) {
  if (value) {
    tjson_emit_string(buf, "true");
  } else {
    tjson_emit_string(buf, "false");
  }
}

void tjson_emit_string(struct tjson_WriteBuffer* buf, const char* str) {
  int count = snprintf(buf->begin, (buf->end - buf->begin), "\"%s\"", str);
  buf->begin += count;
}

void tjson_emit_charbuf(struct tjson_WriteBuffer* buf, const char* begin,
                        const char* end) {
  int count = snprintf(buf->begin, (buf->end - buf->begin), "\"%.*s\"",
                       (int)(begin - end), begin);
  buf->begin += count;
}
