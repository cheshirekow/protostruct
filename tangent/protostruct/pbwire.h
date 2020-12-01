#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#define TANGENT_PBWIRE_VERSION \
  { 0, 1, 1, "dev", 0 }

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARRAY_SIZE
/* GCC is awesome. */
#if __GNUC__
// clang-format off
#define ARRAY_SIZE(arr)                                        \
  (sizeof(arr) / sizeof((arr)[0]) +                            \
    sizeof(typeof(int[1 - 2 * !!__builtin_types_compatible_p(  \
      typeof(arr), typeof(&arr[0]))])) * 0)
// clang-format on
#else
#define ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0])
#endif
#endif

typedef enum pbwire_ErrorCode {
  PBWIRE_NOERROR = 0,
  PBWIRE_INTERNAL_ERROR,   //< bug in the code
  PBWIRE_NOTIMPLEMENTED,   //< request for functionality that is not implemented
                           //  (yet)
  PBWIRE_VARINT_OVERFLOW,  //< received a varint which decoded to more bytes
                           //  than expected
  PBWIRE_VARINT_UNDERFLOW,  //< ran out of bytes while parsing a varint
  PBWIRE_DELIMIT_OVERFLOW,  //< a length-delmited field indicated a size which
                            //< was larger than the available number of bytes
  PBWIRE_VALUE_OVERFLOW,  //< ran out of bytes while parsing a fixed sized value
} pbwire_ErrorCode;

const char* pbwire_ErrorCode_tostring(enum pbwire_ErrorCode value);

// Errors are reported via one of these objects
typedef struct pbwire_Error {
  // Numeric identifier for the error
  enum pbwire_ErrorCode code;

  // Will be filled with a description of the specific error
  char msg[512];
} pbwire_Error;

inline uint32_t pbwire_zigzag32(int32_t value) {
  // NOTE(josh): in python we could do this:
  // return (value << 1) ^ (value >> 31);
  // but in C/C++ we aren't allowed to left shift a signed value
#ifdef __cplusplus
  if (value < 0) {
    return (static_cast<uint32_t>(-value) << 1) - 1;
  } else {
    return static_cast<uint32_t>(value) << 1;
  }
#else
  if (value < 0) {
    return ((uint32_t)(-value) << 1) - 1;
  } else {
    return (uint32_t)(value) << 1;
  }
#endif
}

inline int32_t pbwire_unzigzag32(uint32_t value) {
  return (value >> 1) ^ -(value & 1);
}

inline uint64_t pbwire_zigzag64(int64_t value) {
  // NOTE(josh): in python we could do this:
  // return (value << 1) ^ (value >> 63);
  // but in C/C++ we aren't allowed to left shift a signed value
#ifdef __cplusplus
  if (value < 0) {
    return (static_cast<uint64_t>(-value) << 1) - 1;
  } else {
    return static_cast<uint64_t>(value) << 1;
  }
#else
  if (value < 0) {
    return ((uint64_t)(-value) << 1) - 1;
  } else {
    return (uint64_t)(value) << 1;
  }
#endif
}

inline int32_t pbwire_unzigzag64(uint64_t value) {
  return (value >> 1) ^ -(value & 1);
}

/* ============================ Parsing Funtions ============================ */

typedef struct pbwire_ReadBuffer {
  const char* begin;
  const char* end;
  const char* ptr;
} pbwire_ReadBuffer;

void pbwire_readbuffer_init(pbwire_ReadBuffer* buffer, const char* begin,
                            const char* end);

typedef struct pbwire_ParseContext {
  pbwire_ReadBuffer buffer;
  pbwire_Error* error;
  void* userdata;
} pbwire_ParseContext;

int pbwire_parse_varint32(pbwire_ParseContext* ctx, uint32_t* value);
int pbwire_parse_varint64(pbwire_ParseContext* ctx, uint64_t* value);

inline int pbwire_read_tag(pbwire_ParseContext* ctx, uint32_t* tag) {
  return pbwire_parse_varint32(ctx, tag);
}

typedef int (*pbwire_FieldItemCallback)(pbwire_ParseContext*, void*, uint32_t);

int pbwire_parse_message(pbwire_ParseContext* ctx,
                         pbwire_FieldItemCallback fielditem_callback,
                         void* userdata);

typedef int (*pbwire_RepeatedItemCallback)(pbwire_ParseContext*, void*);

int pbwire_parse_packed_repeated(pbwire_ParseContext* ctx,
                                 pbwire_RepeatedItemCallback item_callback,
                                 void* userdata, size_t object_size,
                                 size_t array_size, size_t* write_idx);

int pbparse_sink_unknown(uint32_t tag, pbwire_ParseContext* ctx);

/* ============================= Emit Funtions ============================== */

typedef struct pbwire_WriteBuffer {
  char* begin;
  char* end;
  char* ptr;
} pbwire_WriteBuffer;

void pbwire_writebuffer_init(pbwire_WriteBuffer* buffer, char* begin,
                             char* end);

/// Stores pre-computed message lenghts in topological order. The lengths
/// are populated by the _pbwire_preemit_XXX() functions and utilized in
/// the _pbwire_emit_XXX() functions.
typedef struct pbwire_LengthCache {
  uint32_t* begin;
  uint32_t* end;
  uint32_t* ptr;
} pbwire_LengthCache;

void pbwire_lengthcache_init(pbwire_LengthCache* buffer, uint32_t* begin,
                             uint32_t* end);

typedef struct pbwire_EmitContext {
  uint32_t passno;
  pbwire_LengthCache length_cache;
  pbwire_WriteBuffer buffer;
  pbwire_Error* error;
  void* userdata;
} pbwire_EmitContext;

int pbwire_emit_varint32(pbwire_EmitContext* ctx, uint32_t value);
int pbwire_emit_varint64(pbwire_EmitContext* ctx, uint64_t value);

static inline int pbwire_write_tag(pbwire_EmitContext* ctx, uint32_t value) {
  return pbwire_emit_varint32(ctx, value);
}

/* ============================= Value Parsers ============================== */

int pbparse_bool(pbwire_ParseContext* ctx, bool* value);
int pbparse_bytes(pbwire_ParseContext* ctx, uint8_t* value, size_t* value_len);
int pbparse_double(pbwire_ParseContext* ctx, double* value);
int pbparse_fixed32(pbwire_ParseContext* ctx, int32_t* value);
int pbparse_fixed64(pbwire_ParseContext* ctx, int64_t* value);
int pbparse_float(pbwire_ParseContext* ctx, float* value);
int pbparse_int8(pbwire_ParseContext* ctx, int8_t* value);
int pbparse_int16(pbwire_ParseContext* ctx, int16_t* value);
int pbparse_int32(pbwire_ParseContext* ctx, int32_t* value);
int pbparse_int64(pbwire_ParseContext* ctx, int64_t* value);
int pbparse_sfixed32(pbwire_ParseContext* ctx, int32_t* value);
int pbparse_sfixed64(pbwire_ParseContext* ctx, int64_t* value);
int pbparse_sint8(pbwire_ParseContext* ctx, int8_t* value);
int pbparse_sint16(pbwire_ParseContext* ctx, int16_t* value);
int pbparse_sint32(pbwire_ParseContext* ctx, int32_t* value);
int pbparse_sint64(pbwire_ParseContext* ctx, int64_t* value);
int pbparse_string(pbwire_ParseContext* ctx, char* value, size_t* value_len);
int pbparse_uint8(pbwire_ParseContext* ctx, uint8_t* value);
int pbparse_uint16(pbwire_ParseContext* ctx, uint16_t* value);
int pbparse_uint32(pbwire_ParseContext* ctx, uint32_t* value);
int pbparse_uint64(pbwire_ParseContext* ctx, uint64_t* value);

/* ============================= Value Emitters ============================= */

int pbemit_bool(pbwire_EmitContext* ctx, bool value);
int pbemit_bytes(pbwire_EmitContext* ctx, uint8_t* value, size_t len);
int pbemit_double(pbwire_EmitContext* ctx, double value);
int pbemit_fixed32(pbwire_EmitContext* ctx, int32_t value);
int pbemit_fixed64(pbwire_EmitContext* ctx, int64_t value);
int pbemit_float(pbwire_EmitContext* ctx, float value);
int pbemit_int8(pbwire_EmitContext* ctx, int8_t value);
int pbemit_int16(pbwire_EmitContext* ctx, int16_t value);
int pbemit_int32(pbwire_EmitContext* ctx, int32_t value);
int pbemit_int64(pbwire_EmitContext* ctx, int64_t value);
int pbemit_sfixed32(pbwire_EmitContext* ctx, int32_t value);
int pbemit_sfixed64(pbwire_EmitContext* ctx, int64_t value);
int pbemit_sin82(pbwire_EmitContext* ctx, int8_t value);
int pbemit_sin162(pbwire_EmitContext* ctx, int16_t value);
int pbemit_sint32(pbwire_EmitContext* ctx, int32_t value);
int pbemit_sint64(pbwire_EmitContext* ctx, int64_t value);
int pbemit_string(pbwire_EmitContext* ctx, char* value, size_t len);
int pbemit_uin82(pbwire_EmitContext* ctx, uint8_t value);
int pbemit_uin162(pbwire_EmitContext* ctx, uint16_t value);
int pbemit_uint32(pbwire_EmitContext* ctx, uint32_t value);
int pbemit_uint64(pbwire_EmitContext* ctx, uint64_t value);

#ifdef __cplusplus
}  // extern "C"
#endif
