// Generated by protostruct. DO NOT EDIT BY HAND!

#include <stdint.h>

#include "tangent/protostruct/test/test_messages.pbwire.h"

#ifdef __cplusplus
extern "C" {
#endif

int pbemit_MyEnumA(pbwire_EmitContext* ctx, MyEnumA value) {
  return pbemit_int32(ctx, (int32_t)value);
}

int pbparse_MyEnumA(pbwire_ParseContext* ctx, MyEnumA* value) {
  int32_t numeric_value;
  int result = pbparse_int32(ctx, &numeric_value);
  if (result < 0) {
    return result;
  }
  switch (numeric_value) {
    case MyEnumA_VALUE1:
      *value = MyEnumA_VALUE1;
      break;
    case MyEnumA_VALUE2:
      *value = MyEnumA_VALUE2;
      break;
    case MyEnumA_VALUE3:
      *value = MyEnumA_VALUE3;
      break;
    default:
      // Invalid enum value, possibly retired. For now let's treat this as
      // an error. In the future, let's figure out a better way of making
      // this recoverable.
      return -1;
  }
  return result;
}

int _pbemit0_MyMessageA(pbwire_EmitContext* ctx, const MyMessageA* obj) {
  int write_result = 0;

  uint32_t encoded_size = 0;
  /* fieldA */
  write_result = pbwire_write_tag(ctx, 8);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_sint32(ctx, obj->fieldA);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldB */
  write_result = pbwire_write_tag(ctx, 17);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_double(ctx, obj->fieldB);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldC */
  write_result = pbwire_write_tag(ctx, 24);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_uint64(ctx, obj->fieldC);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldD */
  write_result = pbwire_write_tag(ctx, 32);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_MyEnumA(ctx, obj->fieldD);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  return encoded_size;
}

int _pbemit1_MyMessageA(pbwire_EmitContext* ctx, const MyMessageA* obj) {
  int write_result = 0;

  char* buffer_begin = ctx->buffer.ptr;
  /* fieldA */
  write_result = pbwire_write_tag(ctx, 8);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_sint32(ctx, obj->fieldA);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldB */
  write_result = pbwire_write_tag(ctx, 17);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_double(ctx, obj->fieldB);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldC */
  write_result = pbwire_write_tag(ctx, 24);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_uint64(ctx, obj->fieldC);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldD */
  write_result = pbwire_write_tag(ctx, 32);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_MyEnumA(ctx, obj->fieldD);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  return (ctx->buffer.ptr - buffer_begin);
}

int pbemit_MyMessageA(pbwire_EmitContext* ctx, const MyMessageA* obj) {
  int retcode = 0;

  retcode = _pbemit0_MyMessageA(ctx, obj);
  if (retcode < 0) {
    return retcode;
  }
  retcode = _pbemit1_MyMessageA(ctx, obj);
  return retcode;
}

static int _parse_fielditem_MyMessageA(pbwire_ParseContext* ctx,
                                       MyMessageA* obj, uint32_t tag) {
  switch (tag) {
    /* fieldA */
    case 8: {
      return pbparse_sint32(ctx, &obj->fieldA);
    }
    /* fieldB */
    case 17: {
      return pbparse_double(ctx, &obj->fieldB);
    }
    /* fieldC */
    case 24: {
      return pbparse_uint64(ctx, &obj->fieldC);
    }
    /* fieldD */
    case 32: {
      return pbparse_MyEnumA(ctx, &obj->fieldD);
    }
    default:
      /* Unknown field */
      return pbparse_sink_unknown(tag, ctx);
  };
}

int pbparse_MyMessageA(pbwire_ParseContext* ctx, MyMessageA* obj) {
  return pbwire_parse_message(
      ctx, (pbwire_FieldItemCallback)_parse_fielditem_MyMessageA, obj);
}

int _pbemit0_MyMessageB(pbwire_EmitContext* ctx, const MyMessageB* obj) {
  int write_result = 0;

  uint32_t* delimit_ptr = NULL;
  int delimit_size = 0;

  uint32_t encoded_size = 0;
  /* fieldA */
  write_result = pbwire_write_tag(ctx, 18);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  delimit_ptr = ctx->length_cache.ptr++;
  write_result = _pbemit0_MyMessageA(ctx, &obj->fieldA);
  if (write_result < 0) {
    return write_result;
  }
  delimit_size = write_result;
  encoded_size += delimit_size;

  *delimit_ptr = delimit_size;
  write_result = pbemit_uint32(ctx, delimit_size);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  return encoded_size;
}

int _pbemit1_MyMessageB(pbwire_EmitContext* ctx, const MyMessageB* obj) {
  int write_result = 0;

  uint32_t* delimit_ptr = NULL;
  int delimit_size = 0;

  char* buffer_begin = ctx->buffer.ptr;
  /* fieldA */
  write_result = pbwire_write_tag(ctx, 18);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  delimit_ptr = ctx->length_cache.ptr++;
  delimit_size = *delimit_ptr;
  write_result = pbemit_uint32(ctx, delimit_size);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = _pbemit1_MyMessageA(ctx, &obj->fieldA);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  return (ctx->buffer.ptr - buffer_begin);
}

int pbemit_MyMessageB(pbwire_EmitContext* ctx, const MyMessageB* obj) {
  int retcode = 0;

  retcode = _pbemit0_MyMessageB(ctx, obj);
  if (retcode < 0) {
    return retcode;
  }
  retcode = _pbemit1_MyMessageB(ctx, obj);
  return retcode;
}

static int _parse_fielditem_MyMessageB(pbwire_ParseContext* ctx,
                                       MyMessageB* obj, uint32_t tag) {
  switch (tag) {
    /* fieldA */
    case 18: {
      return pbparse_MyMessageA(ctx, &obj->fieldA);
    }
    default:
      /* Unknown field */
      return pbparse_sink_unknown(tag, ctx);
  };
}

int pbparse_MyMessageB(pbwire_ParseContext* ctx, MyMessageB* obj) {
  return pbwire_parse_message(
      ctx, (pbwire_FieldItemCallback)_parse_fielditem_MyMessageB, obj);
}

int _pbemit0_MyMessageC(pbwire_EmitContext* ctx, const MyMessageC* obj) {
  int write_result = 0;

  uint32_t* delimit_ptr = NULL;
  int delimit_size = 0;

  uint32_t encoded_size = 0;
  /* fieldA */
  for (int idx = 0; idx < obj->fieldACount; idx++) {
    write_result = pbwire_write_tag(ctx, 10);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;

    delimit_ptr = ctx->length_cache.ptr++;
    write_result = _pbemit0_MyMessageA(ctx, &obj->fieldA[idx]);
    if (write_result < 0) {
      return write_result;
    }
    delimit_size = write_result;
    encoded_size += delimit_size;

    *delimit_ptr = delimit_size;
    write_result = pbemit_uint32(ctx, delimit_size);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;
  }
  /* fieldB */
  write_result = pbwire_write_tag(ctx, 16);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  delimit_size = 0;
  delimit_ptr = ctx->length_cache.ptr++;
  for (int idx = 0; idx < obj->fieldBCount; idx++) {
    write_result = pbemit_int32(ctx, obj->fieldB[idx]);
    if (write_result < 0) {
      return write_result;
    }
    delimit_size += write_result;
  }
  *delimit_ptr = delimit_size;
  encoded_size += write_result;

  write_result = pbemit_uint32(ctx, delimit_size);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldC */
  for (int idx = 0; idx < obj->fieldCCount; idx++) {
    write_result = pbwire_write_tag(ctx, 40);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;

    write_result = pbemit_int32(ctx, obj->fieldC[idx]);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;
  }
  return encoded_size;
}

int _pbemit1_MyMessageC(pbwire_EmitContext* ctx, const MyMessageC* obj) {
  int write_result = 0;

  uint32_t* delimit_ptr = NULL;
  int delimit_size = 0;

  char* buffer_begin = ctx->buffer.ptr;
  /* fieldA */

  for (int idx = 0; idx < obj->fieldACount; idx++) {
    write_result = pbwire_write_tag(ctx, 10);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;

    delimit_ptr = ctx->length_cache.ptr++;
    delimit_size = *delimit_ptr;
    write_result = pbemit_uint32(ctx, delimit_size);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;

    write_result = _pbemit1_MyMessageA(ctx, &obj->fieldA[idx]);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;
  }
  /* fieldB */
  write_result = pbwire_write_tag(ctx, 16);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  delimit_ptr = ctx->length_cache.ptr++;
  delimit_size = *delimit_ptr;
  write_result = pbemit_uint32(ctx, delimit_size);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  for (int idx = 0; idx < obj->fieldBCount; idx++) {
    write_result = pbemit_int32(ctx, obj->fieldB[idx]);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;
  }
  /* fieldC */

  for (int idx = 0; idx < obj->fieldCCount; idx++) {
    write_result = pbwire_write_tag(ctx, 40);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;

    write_result = pbemit_int32(ctx, obj->fieldC[idx]);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;
  }

  return (ctx->buffer.ptr - buffer_begin);
}

int pbemit_MyMessageC(pbwire_EmitContext* ctx, const MyMessageC* obj) {
  int retcode = 0;

  retcode = _pbemit0_MyMessageC(ctx, obj);
  if (retcode < 0) {
    return retcode;
  }
  retcode = _pbemit1_MyMessageC(ctx, obj);
  return retcode;
}

static int _parse_fielditem_MyMessageC(pbwire_ParseContext* ctx,
                                       MyMessageC* obj, uint32_t tag) {
  switch (tag) {
    /* fieldA */
    case 10: {
      return pbparse_MyMessageA(ctx, &obj->fieldA[obj->fieldACount++]);
    }
    /* fieldB */
    case 16: {
      if (obj->fieldBCount < ARRAY_SIZE(obj->fieldB)) {
        return pbparse_int32(ctx, &obj->fieldB[obj->fieldBCount++]);
      } else {
        return pbparse_int32(ctx, NULL);
      }
    }

    /* fieldB (packed) */
    case 18: {
      size_t write_idx = obj->fieldBCount;
      int retcode = pbwire_parse_packed_repeated(
          ctx, (pbwire_RepeatedItemCallback)&pbparse_int32, obj->fieldB,
          sizeof(obj->fieldB[0]), ARRAY_SIZE(obj->fieldB), &write_idx);
      obj->fieldBCount = write_idx;
      return retcode;
    }
    /* fieldC */
    case 40: {
      return pbparse_int32(ctx, &obj->fieldC[obj->fieldCCount++]);
    }
    default:
      /* Unknown field */
      return pbparse_sink_unknown(tag, ctx);
  };
}

int pbparse_MyMessageC(pbwire_ParseContext* ctx, MyMessageC* obj) {
  return pbwire_parse_message(
      ctx, (pbwire_FieldItemCallback)_parse_fielditem_MyMessageC, obj);
}

int _pbemit0_TestFixedArray(pbwire_EmitContext* ctx,
                            const TestFixedArray* obj) {
  int write_result = 0;

  uint32_t encoded_size = 0;
  /* fixedSizedArray */
  for (int idx = 0; idx < ARRAY_SIZE(obj->fixedSizedArray); idx++) {
    write_result = pbwire_write_tag(ctx, 9);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;

    write_result = pbemit_double(ctx, obj->fixedSizedArray[idx]);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;
  }
  return encoded_size;
}

int _pbemit1_TestFixedArray(pbwire_EmitContext* ctx,
                            const TestFixedArray* obj) {
  int write_result = 0;

  char* buffer_begin = ctx->buffer.ptr;
  /* fixedSizedArray */

  for (int idx = 0; idx < ARRAY_SIZE(obj->fixedSizedArray); idx++) {
    write_result = pbwire_write_tag(ctx, 9);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;

    write_result = pbemit_double(ctx, obj->fixedSizedArray[idx]);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;
  }

  return (ctx->buffer.ptr - buffer_begin);
}

int pbemit_TestFixedArray(pbwire_EmitContext* ctx, const TestFixedArray* obj) {
  int retcode = 0;

  retcode = _pbemit0_TestFixedArray(ctx, obj);
  if (retcode < 0) {
    return retcode;
  }
  retcode = _pbemit1_TestFixedArray(ctx, obj);
  return retcode;
}

static int _parse_fielditem_TestFixedArray(pbwire_ParseContext* ctx,
                                           TestFixedArray* obj, uint32_t tag) {
  uint32_t fixedSizedArrayCount = 0;
  switch (tag) {
    /* fixedSizedArray */
    case 9: {
      return pbparse_double(ctx, &obj->fixedSizedArray[fixedSizedArrayCount++]);
    }
    default:
      /* Unknown field */
      return pbparse_sink_unknown(tag, ctx);
  };
}

int pbparse_TestFixedArray(pbwire_ParseContext* ctx, TestFixedArray* obj) {
  return pbwire_parse_message(
      ctx, (pbwire_FieldItemCallback)_parse_fielditem_TestFixedArray, obj);
}

int _pbemit0_TestAlignas(pbwire_EmitContext* ctx, const TestAlignas* obj) {
  int write_result = 0;

  uint32_t encoded_size = 0;
  /* array */
  for (int idx = 0; idx < ARRAY_SIZE(obj->array); idx++) {
    write_result = pbwire_write_tag(ctx, 13);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;

    write_result = pbemit_float(ctx, obj->array[idx]);
    if (write_result < 0) {
      return write_result;
    }
    encoded_size += write_result;
  }
  return encoded_size;
}

int _pbemit1_TestAlignas(pbwire_EmitContext* ctx, const TestAlignas* obj) {
  int write_result = 0;

  char* buffer_begin = ctx->buffer.ptr;
  /* array */

  for (int idx = 0; idx < ARRAY_SIZE(obj->array); idx++) {
    write_result = pbwire_write_tag(ctx, 13);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;

    write_result = pbemit_float(ctx, obj->array[idx]);
    if (write_result < 0) {
      return write_result;
    }
    ctx->buffer.ptr += write_result;
  }

  return (ctx->buffer.ptr - buffer_begin);
}

int pbemit_TestAlignas(pbwire_EmitContext* ctx, const TestAlignas* obj) {
  int retcode = 0;

  retcode = _pbemit0_TestAlignas(ctx, obj);
  if (retcode < 0) {
    return retcode;
  }
  retcode = _pbemit1_TestAlignas(ctx, obj);
  return retcode;
}

static int _parse_fielditem_TestAlignas(pbwire_ParseContext* ctx,
                                        TestAlignas* obj, uint32_t tag) {
  uint32_t arrayCount = 0;
  switch (tag) {
    /* array */
    case 13: {
      return pbparse_float(ctx, &obj->array[arrayCount++]);
    }
    default:
      /* Unknown field */
      return pbparse_sink_unknown(tag, ctx);
  };
}

int pbparse_TestAlignas(pbwire_ParseContext* ctx, TestAlignas* obj) {
  return pbwire_parse_message(
      ctx, (pbwire_FieldItemCallback)_parse_fielditem_TestAlignas, obj);
}

int _pbemit0_TestPrimitives(pbwire_EmitContext* ctx,
                            const TestPrimitives* obj) {
  int write_result = 0;

  uint32_t encoded_size = 0;
  /* fieldA */
  write_result = pbwire_write_tag(ctx, 8);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_int32(ctx, obj->fieldA);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldB */
  write_result = pbwire_write_tag(ctx, 16);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_int32(ctx, obj->fieldB);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldC */
  write_result = pbwire_write_tag(ctx, 24);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_int32(ctx, obj->fieldC);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldD */
  write_result = pbwire_write_tag(ctx, 32);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_int64(ctx, obj->fieldD);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldE */
  write_result = pbwire_write_tag(ctx, 40);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_uint32(ctx, obj->fieldE);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldF */
  write_result = pbwire_write_tag(ctx, 48);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_uint32(ctx, obj->fieldF);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldG */
  write_result = pbwire_write_tag(ctx, 56);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_uint32(ctx, obj->fieldG);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldH */
  write_result = pbwire_write_tag(ctx, 64);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_uint64(ctx, obj->fieldH);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldI */
  write_result = pbwire_write_tag(ctx, 77);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_float(ctx, obj->fieldI);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldJ */
  write_result = pbwire_write_tag(ctx, 81);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_double(ctx, obj->fieldJ);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  /* fieldK */
  write_result = pbwire_write_tag(ctx, 88);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;

  write_result = pbemit_bool(ctx, obj->fieldK);
  if (write_result < 0) {
    return write_result;
  }
  encoded_size += write_result;
  return encoded_size;
}

int _pbemit1_TestPrimitives(pbwire_EmitContext* ctx,
                            const TestPrimitives* obj) {
  int write_result = 0;

  char* buffer_begin = ctx->buffer.ptr;
  /* fieldA */
  write_result = pbwire_write_tag(ctx, 8);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_int32(ctx, obj->fieldA);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldB */
  write_result = pbwire_write_tag(ctx, 16);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_int32(ctx, obj->fieldB);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldC */
  write_result = pbwire_write_tag(ctx, 24);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_int32(ctx, obj->fieldC);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldD */
  write_result = pbwire_write_tag(ctx, 32);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_int64(ctx, obj->fieldD);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldE */
  write_result = pbwire_write_tag(ctx, 40);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_uint32(ctx, obj->fieldE);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldF */
  write_result = pbwire_write_tag(ctx, 48);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_uint32(ctx, obj->fieldF);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldG */
  write_result = pbwire_write_tag(ctx, 56);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_uint32(ctx, obj->fieldG);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldH */
  write_result = pbwire_write_tag(ctx, 64);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_uint64(ctx, obj->fieldH);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldI */
  write_result = pbwire_write_tag(ctx, 77);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_float(ctx, obj->fieldI);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldJ */
  write_result = pbwire_write_tag(ctx, 81);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_double(ctx, obj->fieldJ);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;
  /* fieldK */
  write_result = pbwire_write_tag(ctx, 88);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  write_result = pbemit_bool(ctx, obj->fieldK);
  if (write_result < 0) {
    return write_result;
  }
  ctx->buffer.ptr += write_result;

  return (ctx->buffer.ptr - buffer_begin);
}

int pbemit_TestPrimitives(pbwire_EmitContext* ctx, const TestPrimitives* obj) {
  int retcode = 0;

  retcode = _pbemit0_TestPrimitives(ctx, obj);
  if (retcode < 0) {
    return retcode;
  }
  retcode = _pbemit1_TestPrimitives(ctx, obj);
  return retcode;
}

static int _parse_fielditem_TestPrimitives(pbwire_ParseContext* ctx,
                                           TestPrimitives* obj, uint32_t tag) {
  switch (tag) {
    /* fieldA */
    case 8: {
      return pbparse_int8(ctx, &obj->fieldA);
    }
    /* fieldB */
    case 16: {
      return pbparse_int16(ctx, &obj->fieldB);
    }
    /* fieldC */
    case 24: {
      return pbparse_int32(ctx, &obj->fieldC);
    }
    /* fieldD */
    case 32: {
      return pbparse_int64(ctx, &obj->fieldD);
    }
    /* fieldE */
    case 40: {
      return pbparse_uint8(ctx, &obj->fieldE);
    }
    /* fieldF */
    case 48: {
      return pbparse_uint16(ctx, &obj->fieldF);
    }
    /* fieldG */
    case 56: {
      return pbparse_uint32(ctx, &obj->fieldG);
    }
    /* fieldH */
    case 64: {
      return pbparse_uint64(ctx, &obj->fieldH);
    }
    /* fieldI */
    case 77: {
      return pbparse_float(ctx, &obj->fieldI);
    }
    /* fieldJ */
    case 81: {
      return pbparse_double(ctx, &obj->fieldJ);
    }
    /* fieldK */
    case 88: {
      return pbparse_bool(ctx, &obj->fieldK);
    }
    default:
      /* Unknown field */
      return pbparse_sink_unknown(tag, ctx);
  };
}

int pbparse_TestPrimitives(pbwire_ParseContext* ctx, TestPrimitives* obj) {
  return pbwire_parse_message(
      ctx, (pbwire_FieldItemCallback)_parse_fielditem_TestPrimitives, obj);
}

#ifdef __cplusplus
}  // extern "C"
#endif
