// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/protostruct/pbwire.h"

#include <cassert>
#include <cstring>
#include <type_traits>

#include "tangent/util/fixed_string_stream.h"

typedef enum pbwire_WireType {
  PBWIRE_WIRETYPE_VARINT = 0,
  PBWIRE_WIRETYPE_FIXED64 = 1,
  PBWIRE_WIRETYPE_LENGTH_DELIMITED = 2,
  PBWIRE_WIRETYPE_START_GROUP = 3,
  PBWIRE_WIRETYPE_END_GROUP = 4,
  PBWIRE_WIRETYPE_FIXED32 = 5
} pbwire_WireType;

util::FixedCharStream pbwire_error(pbwire_Error* err, pbwire_ErrorCode code) {
  if (!err) {
    return util::FixedCharStream(0, static_cast<size_t>(0));
  }
  err->code = code;
  return util::FixedCharStream(err->msg, sizeof(err->msg));
}

template <typename T>
inline typename std::make_unsigned<T>::type _zigzag(T value) {
  using uT = typename std::make_unsigned<T>::type;
  // NOTE(josh): in python we could do this:
  // return (value << 1) ^ (value >> (8 * sizeof(T) - 1));
  // but in C++ we aren't allowed to left shift a signed value
  if (value < 0) {
    return (static_cast<uT>(-value) << 1) - 1;
  } else {
    return static_cast<uT>(value) << 1;
  }
}

template <typename T>
inline typename std::make_signed<T>::type _unzigzag(T value) {
  return (value >> 1) ^ -(value & 1);
}

/* ============================ Parsing Funtions ============================ */

void pbwire_readbuffer_init(pbwire_ReadBuffer* buffer, const char* begin,
                            const char* end) {
  buffer->begin = begin;
  buffer->ptr = begin;
  buffer->end = end;
}

template <typename T>
int _parse_uvarint(pbwire_ParseContext* ctx, T* value_out) {
  static_assert(std::is_unsigned<T>::value,
                "parse_varint<T> can only be used for unsigned types");
  constexpr size_t max_bytes = ((sizeof(T) * 8) + 7 - 1) / 7;
  const char mask = 0x7f;
  const char morebit = 0x80;
  size_t byte_idx = 0;
  size_t bufcount = ctx->buffer.end - ctx->buffer.ptr;
  size_t itercount = std::min<size_t>(bufcount, max_bytes);
  T value = 0;

#if __GNUC__ > 8
#pragma GCC unroll 10
#endif
  for (byte_idx = 0; byte_idx < itercount; byte_idx++) {
    (value) |= (ctx->buffer.ptr[byte_idx] & mask) << (7 * byte_idx);

    // If the most significant bit is zero, then this is the last byte in
    // the encoding, so just return the number of bytes read.
    if ((ctx->buffer.ptr[byte_idx] & morebit) == 0) {
      if (value_out) {
        *value_out = value;
      }
      return byte_idx + 1;
    }
  }

  if (byte_idx == bufcount) {
    pbwire_error(ctx->error, PBWIRE_VARINT_UNDERFLOW)
        << "buffer expired after " << (ctx->buffer.end - ctx->buffer.begin)
        << " bytes, but last byte had the more bit set";
    return -1;
  }

  pbwire_error(ctx->error, PBWIRE_VARINT_OVERFLOW)
      << "got " << byte_idx << " bytes while parsing a 32 bit number (max "
      << max_bytes << " bytes)";
  return -1;
}

// parse a possibly signed integer, which is encoded as an unsigned integer
template <typename T>
inline int _parse_svarint(pbwire_ParseContext* ctx, T* value_out) {
  typename std::make_unsigned<T>::type value;
  int bytes_read = _parse_uvarint(ctx, &value);
  if (value_out) {
    memcpy(value_out, &value, sizeof(T));
  }
  return bytes_read;
}

// parse zig-zag signed integer
template <typename T>
inline int _parse_zvarint(pbwire_ParseContext* ctx, T* value_out) {
  typename std::make_unsigned<T>::type value = 0;
  int bytes_read = _parse_uvarint(ctx, &value);
  if (bytes_read > 0 && value_out) {
    *value_out = _unzigzag(value);
  }
  return bytes_read;
}

template <typename T>
int _parse_fixed(pbwire_ParseContext* ctx, T* value_out) {
  if (ctx->buffer.ptr + sizeof(T) > ctx->buffer.end) {
    pbwire_error(ctx->error, PBWIRE_VALUE_OVERFLOW)
        << "got " << (ctx->buffer.end - ctx->buffer.ptr)
        << " bytes while parsing a value that requires " << sizeof(T);
    return -1;
  }
  if (value_out) {
    memcpy(value_out, ctx->buffer.ptr, sizeof(T));
  }
  return sizeof(T);
}

template <typename T>
int _parse_delimited(pbwire_ParseContext* ctx, T* value_out,
                     size_t* value_len) {
  uint32_t length = 0;
  int bytes_read = _parse_uvarint(ctx, &length);
  if (bytes_read < 0) {
    return bytes_read;
  }
  ctx->buffer.ptr += bytes_read;
  if (ctx->buffer.ptr + length > ctx->buffer.end) {
    pbwire_error(ctx->error, PBWIRE_DELIMIT_OVERFLOW)
        << "Read a delmited length of " << length << " but only have "
        << (ctx->buffer.end - ctx->buffer.ptr) << " bytes left";
  }

  if (value_out) {
    assert(value_len);
    if (length < *value_len) {
      *value_len = length;
    }
    // TODO(josh): warn overflow?

    memcpy(value_out, ctx->buffer.ptr, *value_len);
  }

  return bytes_read + length;
}

int pbwire_parse_varint32(pbwire_ParseContext* ctx, uint32_t* value) {
  return _parse_uvarint(ctx, value);
}

int pbwire_parse_varint64(pbwire_ParseContext* ctx, uint64_t* value) {
  return _parse_uvarint(ctx, value);
}

/*
 * From: https://developers.google.com/protocol-buffers/docs/encoding#embedded
 *
 *   A wire type of 2 (length-delimited) means that the value is a varint
 *   encoded length followed by the specified number of bytes of data.
 *
 *   ...
 *
 *   As you can see, the last three bytes are exactly the same as our first
 *   example (08 96 01), and they're preceded by the number 3 – embedded
 *   messages are treated in exactly the same way as strings (wire type = 2).
 */
int pbwire_parse_message(pbwire_ParseContext* ctx,
                         pbwire_FieldItemCallback fielditem_callback,
                         void* userdata) {
  int bytes_read = 0;
  uint32_t tag = 0;
  uint32_t size_delimit = 0;

  const char* buffer_begin = ctx->buffer.ptr;
  while (ctx->buffer.ptr != ctx->buffer.end) {
    bytes_read = pbparse_uint32(ctx, &tag);
    if (bytes_read < 0) {
      return bytes_read;
    }
    ctx->buffer.ptr += bytes_read;

    pbwire_ParseContext sub_ctx{};
    sub_ctx.buffer = ctx->buffer;
    sub_ctx.error = ctx->error;

    uint32_t wire_type = (tag & 0x7);
    if (wire_type == PBWIRE_WIRETYPE_LENGTH_DELIMITED) {
      bytes_read = pbwire_parse_varint32(ctx, &size_delimit);
      if (bytes_read < 0) {
        return bytes_read;
      }
      ctx->buffer.ptr += bytes_read;
      sub_ctx.buffer.ptr = ctx->buffer.ptr;
      sub_ctx.buffer.begin = sub_ctx.buffer.ptr;
      sub_ctx.buffer.end = sub_ctx.buffer.begin + size_delimit;
    } else {
      sub_ctx.buffer.begin = sub_ctx.buffer.ptr;
    }

    bytes_read = fielditem_callback(&sub_ctx, userdata, tag);
    if (bytes_read < 0) {
      return bytes_read;
    }
    ctx->buffer.ptr += bytes_read;
  }
  return (ctx->buffer.end - buffer_begin);
}

/*
 * From: https://developers.google.com/protocol-buffers/docs/encoding#types:
 *
 *   Version 2.1.0 introduced packed repeated fields, which in proto2 are
 *   declared like repeated fields but with the special [packed=true] option. In
 *   proto3, repeated fields of scalar numeric types are packed by default.
 *   These function like repeated fields, but are encoded differently. A packed
 *   repeated field containing zero elements does not appear in the encoded
 *   message. Otherwise, all of the elements of the field are packed into a
 *   single key-value pair with wire type 2 (length-delimited). Each element is
 *   encoded the same way it would be normally, except without a key preceding
 *   it.
 */
int pbwire_parse_packed_repeated(pbwire_ParseContext* ctx,
                                 pbwire_RepeatedItemCallback item_callback,
                                 void* userdata, size_t object_size,
                                 size_t array_size, size_t* write_idx) {
  int bytes_read = 0;
  uint32_t bytes_packed = ctx->buffer.end - ctx->buffer.begin;
  char* write_ptr = reinterpret_cast<char*>(userdata);

  for (*write_idx = 0; *write_idx < array_size; (*write_idx)++) {
    bytes_read = item_callback(ctx, write_ptr);
    if (bytes_read < 0) {
      return bytes_read;
    }

    ctx->buffer.ptr += bytes_read;
    if (ctx->buffer.ptr >= ctx->buffer.end) {
      return static_cast<int>(bytes_packed);
    }

    if (write_ptr) {
      write_ptr += object_size;
    }
  }
  return bytes_packed;
}

int pbparse_sink_unknown(uint32_t tag, pbwire_ParseContext* ctx) {
  switch (tag & 0x7) {
    // wire-type
    case PBWIRE_WIRETYPE_VARINT: {
      uint64_t dummy = 0;
      return _parse_uvarint(ctx, &dummy);
    }

    case PBWIRE_WIRETYPE_FIXED64: {
      int64_t dummy = 0;
      return pbparse_fixed64(ctx, &dummy);
    }

    case PBWIRE_WIRETYPE_FIXED32: {
      int32_t dummy = 0;
      return pbparse_fixed32(ctx, &dummy);
    }

    case PBWIRE_WIRETYPE_LENGTH_DELIMITED: {
      uint32_t length = 0;
      int varint_bytes = pbwire_parse_varint32(ctx, &length);
      if (varint_bytes < 0) {
        return varint_bytes;
      }
      return varint_bytes + length;
    }

    default:
      return -1;
  }
}

/* ============================= Emit Funtions ============================== */

void pbwire_writebuffer_init(pbwire_WriteBuffer* buffer, char* begin,
                             char* end) {
  buffer->begin = begin;
  buffer->ptr = begin;
  buffer->end = end;
}

void pbwire_lengthcache_init(pbwire_LengthCache* length_cache, uint32_t* begin,
                             uint32_t* end) {
  length_cache->begin = begin;
  length_cache->ptr = begin;
  length_cache->end = end;
}

template <typename T>
int _emit_uvarint(pbwire_EmitContext* ctx, T value) {
  static_assert(std::is_unsigned<T>::value,
                "_emit_varint<T> can only be used for unsigned types");
  const char mask = 0x7f;
  const char morebit = 0x80;
  size_t byte_idx = 0;
  size_t bufcount = ctx->buffer.end - ctx->buffer.ptr;

#if __GNUC__ > 8
#pragma GCC unroll 10
#endif
  for (byte_idx = 0; byte_idx < bufcount; byte_idx++) {
    ctx->buffer.ptr[byte_idx] = (value >> (7 * byte_idx)) & mask;

    if ((value >> (7 * (byte_idx + 1)))) {
      // If the remaining bytes contain nonzero content then set the
      // morebit.
      ctx->buffer.ptr[byte_idx] |= morebit;
    } else {
      // If all remaining bytes would be empty, then we are done. Leave the
      // more bit clear and return the number of bytes written
      return byte_idx + 1;
    }
  }

  pbwire_error(ctx->error, PBWIRE_VARINT_OVERFLOW)
      << "buffer expired after " << bufcount
      << " bytes, but we had more bytes to write";
  return -1;
}

template <typename T>
inline int _emit_svarint(pbwire_EmitContext* ctx, T value) {
  typename std::make_unsigned<T>::type uvalue;
  memcpy(&uvalue, &value, sizeof(T));
  return _emit_uvarint(ctx, uvalue);
}

template <typename T>
int _emit_fixed(pbwire_EmitContext* ctx, T value) {
  if (ctx->buffer.ptr + sizeof(T) > ctx->buffer.end) {
    pbwire_error(ctx->error, PBWIRE_VALUE_OVERFLOW)
        << "buffer only has " << (ctx->buffer.end - ctx->buffer.ptr)
        << " bytes left, and need to write " << sizeof(T);
    return -1;
  }
  memcpy(ctx->buffer.ptr, &value, sizeof(T));
  return sizeof(T);
}

template <typename T>
int _emit_delimited(pbwire_EmitContext* ctx, T* value, size_t value_len) {
  int bytes_written = _emit_uvarint(ctx, value_len);
  if (bytes_written < 0) {
    return bytes_written;
  }
  ctx->buffer.ptr += bytes_written;
  if (ctx->buffer.ptr + value_len > ctx->buffer.end) {
    pbwire_error(ctx->error, PBWIRE_VALUE_OVERFLOW)
        << "buffer only has " << (ctx->buffer.end - ctx->buffer.ptr)
        << " bytes left, and need to write " << value_len;
    return -1;
  }
  memcpy(ctx->buffer.ptr, value, value_len * sizeof(T));
  return value_len * sizeof(T);
}

int pbwire_emit_varint32(pbwire_EmitContext* ctx, uint32_t value) {
  return _emit_uvarint(ctx, value);
}

int pbwire_emit_varint64(pbwire_EmitContext* ctx, uint64_t value) {
  return _emit_uvarint(ctx, value);
}

/* ============================= Value Parsers ============================== */

int pbparse_bool(pbwire_ParseContext* ctx, bool* value) {
  int8_t ivalue = 0;
  int bytes_read = _parse_svarint(ctx, &ivalue);
  *value = static_cast<bool>(ivalue);
  return bytes_read;
}

int pbparse_bytes(pbwire_ParseContext* ctx, uint8_t* value, size_t* value_len) {
  return _parse_delimited(ctx, value, value_len);
}

int pbparse_double(pbwire_ParseContext* ctx, double* value) {
  return _parse_fixed(ctx, value);
}

int pbparse_fixed32(pbwire_ParseContext* ctx, int32_t* value) {
  return _parse_fixed(ctx, value);
}

int pbparse_fixed64(pbwire_ParseContext* ctx, int64_t* value) {
  return _parse_fixed(ctx, value);
}

int pbparse_float(pbwire_ParseContext* ctx, float* value) {
  return _parse_fixed(ctx, value);
}

int pbparse_int8(pbwire_ParseContext* ctx, int8_t* value) {
  return _parse_svarint(ctx, value);
}

int pbparse_int16(pbwire_ParseContext* ctx, int16_t* value) {
  return _parse_svarint(ctx, value);
}

int pbparse_int32(pbwire_ParseContext* ctx, int32_t* value) {
  return _parse_svarint(ctx, value);
}

int pbparse_int64(pbwire_ParseContext* ctx, int64_t* value) {
  return _parse_svarint(ctx, value);
}

int pbparse_sfixed32(pbwire_ParseContext* ctx, int32_t* value) {
  return _parse_fixed(ctx, value);
}

int pbparse_sfixed64(pbwire_ParseContext* ctx, int64_t* value) {
  return _parse_fixed(ctx, value);
}

int pbparse_sint8(pbwire_ParseContext* ctx, int8_t* value) {
  return _parse_zvarint(ctx, value);
}

int pbparse_sint16(pbwire_ParseContext* ctx, int16_t* value) {
  return _parse_zvarint(ctx, value);
}

int pbparse_sint32(pbwire_ParseContext* ctx, int32_t* value) {
  return _parse_zvarint(ctx, value);
}

int pbparse_sint64(pbwire_ParseContext* ctx, int64_t* value) {
  return _parse_zvarint(ctx, value);
}

int pbparse_string(pbwire_ParseContext* ctx, char* value, size_t* value_len) {
  return _parse_delimited(ctx, value, value_len);
}

int pbparse_uint8(pbwire_ParseContext* ctx, uint8_t* value) {
  return _parse_uvarint(ctx, value);
}

int pbparse_uint16(pbwire_ParseContext* ctx, uint16_t* value) {
  return _parse_uvarint(ctx, value);
}

int pbparse_uint32(pbwire_ParseContext* ctx, uint32_t* value) {
  return _parse_uvarint(ctx, value);
}

int pbparse_uint64(pbwire_ParseContext* ctx, uint64_t* value) {
  return _parse_uvarint(ctx, value);
}

/* ============================= Value Emitters ============================= */

int pbemit_bool(pbwire_EmitContext* ctx, bool value) {
  return _emit_uvarint(ctx, static_cast<uint8_t>(value));
}

int pbemit_bytes(pbwire_EmitContext* ctx, uint8_t* value, size_t len) {
  return _emit_delimited(ctx, value, len);
}

int pbemit_double(pbwire_EmitContext* ctx, double value) {
  return _emit_fixed(ctx, value);
}

int pbemit_fixed32(pbwire_EmitContext* ctx, int32_t value) {
  return _emit_fixed(ctx, value);
}

int pbemit_fixed64(pbwire_EmitContext* ctx, int64_t value) {
  return _emit_fixed(ctx, value);
}

int pbemit_float(pbwire_EmitContext* ctx, float value) {
  return _emit_fixed(ctx, value);
}

int pbemit_int8(pbwire_EmitContext* ctx, int8_t value) {
  return _emit_svarint(ctx, value);
}

int pbemit_int16(pbwire_EmitContext* ctx, int16_t value) {
  return _emit_svarint(ctx, value);
}

int pbemit_int32(pbwire_EmitContext* ctx, int32_t value) {
  return _emit_svarint(ctx, value);
}

int pbemit_int64(pbwire_EmitContext* ctx, int64_t value) {
  return _emit_svarint(ctx, value);
}

int pbemit_sfixed32(pbwire_EmitContext* ctx, int32_t value) {
  return _emit_fixed(ctx, value);
}

int pbemit_sfixed64(pbwire_EmitContext* ctx, int64_t value) {
  return _emit_fixed(ctx, value);
}

int pbemit_sint8(pbwire_EmitContext* ctx, int8_t value) {
  return _emit_uvarint(ctx, _zigzag(value));
}

int pbemit_sint16(pbwire_EmitContext* ctx, int16_t value) {
  return _emit_uvarint(ctx, _zigzag(value));
}

int pbemit_sint32(pbwire_EmitContext* ctx, int32_t value) {
  return _emit_uvarint(ctx, _zigzag(value));
}

int pbemit_sint64(pbwire_EmitContext* ctx, int64_t value) {
  return _emit_uvarint(ctx, _zigzag(value));
}

int pbemit_string(pbwire_EmitContext* ctx, char* value, size_t len) {
  return _emit_delimited(ctx, value, len);
}

int pbemit_uint8(pbwire_EmitContext* ctx, uint8_t value) {
  return _emit_uvarint(ctx, value);
}

int pbemit_uint16(pbwire_EmitContext* ctx, uint16_t value) {
  return _emit_uvarint(ctx, value);
}

int pbemit_uint32(pbwire_EmitContext* ctx, uint32_t value) {
  return _emit_uvarint(ctx, value);
}

int pbemit_uint64(pbwire_EmitContext* ctx, uint64_t value) {
  return _emit_uvarint(ctx, value);
}
