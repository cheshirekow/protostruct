#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <stdint.h>

#include "tangent/tjson/tjson.h"

// -----------------------------------------------------------------------------
//    Output buffer for document generation
// -----------------------------------------------------------------------------

struct tjson_WriteBuffer {
  char* begin;
  char* end;
};

// -----------------------------------------------------------------------------
//    Emit Helpers
// -----------------------------------------------------------------------------
// These implement reusable algorithms that are used for multiple different
// types

void tjson_emit_uint64(struct tjson_WriteBuffer* buf, uint64_t value);
void tjson_emit_uint32(struct tjson_WriteBuffer* buf, uint32_t value);
void tjson_emit_uint16(struct tjson_WriteBuffer* buf, uint16_t value);
void tjson_emit_uint8(struct tjson_WriteBuffer* buf, uint8_t value);

void tjson_emit_int64(struct tjson_WriteBuffer* buf, int64_t value);
void tjson_emit_int32(struct tjson_WriteBuffer* buf, int32_t value);
void tjson_emit_int16(struct tjson_WriteBuffer* buf, int16_t value);
void tjson_emit_int8(struct tjson_WriteBuffer* buf, int8_t valu);

void tjson_emit_double(struct tjson_WriteBuffer* buf, double value);
void tjson_emit_float(struct tjson_WriteBuffer* buf, double value);

void tjson_emit_boolean(struct tjson_WriteBuffer* buf, int8_t value);

void tjson_emit_string(struct tjson_WriteBuffer* buf, const char* str);
void tjson_emit_charbuf(struct tjson_WriteBuffer* buf, const char* begin,
                        const char* end);
