#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <stdint.h>

#include "tangent/tjson/tjson.h"

#if __cplusplus
extern "C" {
#endif

typedef struct tjson_ParseContext {
  tjson_LexerParser* stream;
  tjson_Error* error;
  void* userdata;
} tjson_ParseContext;

/// Escape any control codes to json shortcode,
ssize_t tjson_escape(const struct tjson_StringPiece input, char* begin,
                     char* end);

/// Unescape any JSON encoded control codes, reconstruct the original string
/// in the character buffer pointed to by (begin,end). Return the number of
/// characters in the output (not including the null terminator).
ssize_t tjson_unescape(const struct tjson_StringPiece input, char* begin,
                       char* end);

// -----------------------------------------------------------------------------
//    Parse Helpers
// -----------------------------------------------------------------------------
// These implement reusable algorithms that are used for multiple different
// types

int tjson_parse_value_uint64(const struct tjson_Token* token, uint64_t* value,
                             struct tjson_Error* error);
int tjson_parse_value_uint32(const struct tjson_Token* token, uint32_t* value,
                             struct tjson_Error* error);
int tjson_parse_value_uint16(const struct tjson_Token* token, uint16_t* value,
                             struct tjson_Error* error);
int tjson_parse_value_uint8(const struct tjson_Token* token, uint8_t* value,
                            struct tjson_Error* error);

int tjson_parse_value_int64(const struct tjson_Token* token, int64_t* value,
                            struct tjson_Error* error);
int tjson_parse_value_int32(const struct tjson_Token* token, int32_t* value,
                            struct tjson_Error* error);
int tjson_parse_value_int16(const struct tjson_Token* token, int16_t* value,
                            struct tjson_Error* error);
int tjson_parse_value_int8(const struct tjson_Token* token, int8_t* value,
                           struct tjson_Error* error);

int tjson_parse_value_double(const struct tjson_Token* token, double* value,
                             struct tjson_Error* error);
int tjson_parse_value_float(const struct tjson_Token* token, float* value,
                            struct tjson_Error* error);

int tjson_parse_value_boolean(const struct tjson_Token* token, int8_t* value,
                              struct tjson_Error* error);

int tjson_parse_value_string(const struct tjson_Token* token, char* buf,
                             size_t buflen, struct tjson_Error* error);

// -----------------------------------------------------------------------------
//    Value Parsers
// -----------------------------------------------------------------------------

int tjson_parse_uint64(tjson_ParseContext ctx, uint64_t* value);
int tjson_parse_uint32(tjson_ParseContext ctx, uint32_t* value);
int tjson_parse_uint16(tjson_ParseContext ctx, uint16_t* value);
int tjson_parse_uint8(tjson_ParseContext ctx, uint8_t* value);

int tjson_parse_int64(tjson_ParseContext ctx, int64_t* value);
int tjson_parse_int32(tjson_ParseContext ctx, int32_t* value);
int tjson_parse_int16(tjson_ParseContext ctx, int16_t* value);
int tjson_parse_int8(tjson_ParseContext ctx, int8_t* value);

int tjson_parse_double(tjson_ParseContext ctx, double* value);
int tjson_parse_float(tjson_ParseContext ctx, float* value);

int tjson_parse_boolean(tjson_ParseContext ctx, int8_t* value);

int tjson_parse_string(tjson_ParseContext ctx, char* buf, size_t buflen);

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------
// Use these to consume a coherent event-group from the LexerParser without
// actually looking at the data.

// Consume a value, ignoring it's contents
int tjson_sink_event(tjson_ParseContext ctx, const struct tjson_Event* event);

int tjson_sink_value(tjson_ParseContext ctx);

// Consume an object, ignoring it's contents
int tjson_sink_object(tjson_ParseContext ctx, int8_t already_open);

// Consume a list, ignoring it's contents
int tjson_sink_list(tjson_ParseContext ctx, int8_t already_open);

// -----------------------------------------------------------------------------
//    Decoders
// -----------------------------------------------------------------------------

typedef int (*tjson_fielditem_callback)(void*, tjson_ParseContext,
                                        tjson_StringPiece);

int tjson_parse_object(tjson_ParseContext ctx,
                       int (*fielditem_callback)(void*, tjson_ParseContext,
                                                 tjson_StringPiece),
                       void* userdata);

typedef int (*tjson_listitem_callback)(void*, tjson_ParseContext);

int tjson_parse_list(tjson_ParseContext ctx,
                     int (*listitem_callback)(void*, tjson_ParseContext),
                     void* userdata);

int tjson_count_list(tjson_ParseContext ctx, size_t* count);

#if __cplusplus
}  // extern "C"
#endif
