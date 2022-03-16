// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/tjson/parse.h"

#include <stdio.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
//    Util
// -----------------------------------------------------------------------------

// Donald Knuth's hash function
// TODO(josh): validate or replace this hash function
// static inline uint64_t stringhash(char* ptr, uint64_t hashv) {
//   return (*ptr == '\0')
//              ? hashv
//              : stringhash(ptr + 1, ((hashv << 5) ^ (hashv >> 27)) ^ *ptr);
// }

/// Return true if the given character is a control code
static int32_t is_control_code(char code) {
  return ('\x00' <= code && code <= '\x1f');
}

struct CodePair {
  char key;
  char value;
};

/// Map control codes that have a JSON shortcode to their shortcode
static const struct CodePair kEscapeMap[] = {
    // clang-format off
    {'"', '"'},
    {'\\', '\\'},
    {'\b', 'b'},
    {'\f', 'f'},
    {'\n', 'n'},
    {'\r', 'r'},
    {'\t', 't'},
    {0, 0}
    // clang-format on
};

static const struct CodePair* map_find_key(const struct CodePair* map,
                                           char key) {
  for (; map->key; map++) {
    if (map->key == key) {
      return map;
    }
  }
  return NULL;
}

static const struct CodePair* map_find_value(const struct CodePair* map,
                                             char value) {
  for (; map->value; map++) {
    if (map->value == value) {
      return map;
    }
  }
  return NULL;
}

// adapted from: https://stackoverflow.com/a/33799784/141023
ssize_t tjson_escape(const struct tjson_StringPiece input, char* begin,
                     char* end) {
  char* out = begin;
  for (const char* ptr = input.begin; ptr != input.end; ptr++) {
    const struct CodePair* pair = map_find_key(kEscapeMap, *ptr);
    if (pair) {
      if (out + 2 >= end) {
        return -1;
      }
      (*out++) = '\\';
      (*out++) = pair->value;
    } else if (is_control_code(*ptr)) {
      if (out + 6 >= end) {
        return -1;
      }
      if (snprintf(out, end - out, "\\u%04x", (int)(*ptr)) != 6) {
        return -1;
      }
    } else {
      if (out + 1 >= end) {
        return -1;
      }
      (*out++) = *ptr;
    }
  }

  return 0;
}

ssize_t tjson_unescape(const struct tjson_StringPiece input, char* begin,
                       char* end) {
  char* out = begin;
  for (const char* ptr = input.begin; ptr != input.end; ++ptr) {
    if (out + 1 >= end) {
      return -1;
    }
    if (ptr + 1 == input.end) {
      (*out++) = *ptr;
      continue;
    }
    if (ptr[0] == '\\') {
      const struct CodePair* pair = map_find_value(kEscapeMap, ptr[1]);
      if (pair) {
        (*out++) = pair->key;
        ptr++;
      } else if (ptr[1] == 'u' && ptr + 5 < input.end) {
        char buffer[5];
        for (size_t idx = 0; idx < 4; idx++) {
          buffer[idx] = ptr[idx + 2];
        }
        buffer[4] = '\0';
        (*out++) = (char)strtol(buffer, NULL, 16);
        ptr += 5;
      } else {
        (*out++) = *ptr;
      }
    } else {
      (*out++) = *ptr;
    }
  }
  return out - begin;
}

// -----------------------------------------------------------------------------
//    Parser Implementations
// -----------------------------------------------------------------------------

static inline int assert_numeric(const struct tjson_Token* token,
                                 struct tjson_Error* error) {
  if (token->typeno != TJSON_NUMERIC_LITERAL) {
    error->code = TJSON_WRONG_PRIMITIVE;
    error->loc = token->location;
    snprintf(error->msg, sizeof(error->msg),
             "Can't parse token of type %s as a number",
             tjson_TokenTypeNo_tostring(token->typeno));
    return -1;
  }
  return 0;
}

int tjson_parse_value_uint64(const struct tjson_Token* token, uint64_t* value,
                             struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = strtoull(token->spelling.begin, NULL, 0);
  return 0;
}

int tjson_parse_value_uint32(const struct tjson_Token* token, uint32_t* value,
                             struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = strtoul(token->spelling.begin, NULL, 0);
  return 0;
}
int tjson_parse_value_uint16(const struct tjson_Token* token, uint16_t* value,
                             struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = (uint16_t)strtoul(token->spelling.begin, NULL, 0);
  return 0;
}
int tjson_parse_value_uint8(const struct tjson_Token* token, uint8_t* value,
                            struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = (uint8_t)strtoul(token->spelling.begin, NULL, 0);
  return 0;
}

int tjson_parse_value_int64(const struct tjson_Token* token, int64_t* value,
                            struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = strtoll(token->spelling.begin, NULL, 0);
  return 0;
}

int tjson_parse_value_int32(const struct tjson_Token* token, int32_t* value,
                            struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = strtol(token->spelling.begin, NULL, 0);
  return 0;
}
int tjson_parse_value_int16(const struct tjson_Token* token, int16_t* value,
                            struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = (int16_t)strtol(token->spelling.begin, NULL, 0);
  return 0;
}
int tjson_parse_value_int8(const struct tjson_Token* token, int8_t* value,
                           struct tjson_Error* error) {
  if (assert_numeric(token, error)) {
    return -1;
  }

  // NOTE(josh): all well-formed json will have a valid terminating character,
  // and we wouldn't have gotten to this point if it didn't.
  *value = (int8_t)strtol(token->spelling.begin, NULL, 0);
  return 0;
}

int tjson_parse_value_double(const struct tjson_Token* token, double* value,
                             struct tjson_Error* error) {
  *value = strtod(token->spelling.begin, NULL);
  return 0;
}

int tjson_parse_value_float(const struct tjson_Token* token, float* value,
                            struct tjson_Error* error) {
  *value = strtof(token->spelling.begin, NULL);
  return 0;
}

static const char* kTrueStrings[] = {"y",    "Y",    "yes", "YES", "t", "T",
                                     "true", "TRUE", "on",  "ON",  "1"};

static const char* kFalseStrings[] = {"n",     "N",     "no",  "NO",  "f", "F",
                                      "false", "FALSE", "off", "OFF", "0"};

int tjson_parse_value_boolean(const struct tjson_Token* token, int8_t* value,
                              struct tjson_Error* error) {
  switch (token->typeno) {
    case TJSON_BOOLEAN_LITERAL: {
      if (tjson_StringPiece_streq(token->spelling, "true")) {
        (*value) = 1;
        return 0;
      } else if (tjson_StringPiece_streq(token->spelling, "false")) {
        (*value) = 0;
        return 0;
      } else {
        // Lexer shouldn't match a BOOLEAN_LITERAL for any other strings.
        error->code = TJSON_WRONG_PRIMITIVE;
        error->loc = token->location;
        snprintf(error->msg, sizeof(error->msg),
                 "Unexpected boolean literal %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);
        return -1;
      }
      break;
    }

    case TJSON_STRING_LITERAL: {
      for (size_t idx = 0; idx < sizeof(kTrueStrings) / sizeof(char*); idx++) {
        if (tjson_StringPiece_streq(token->spelling, kTrueStrings[idx])) {
          *value = 1;
          return 0;
        }
      }
      for (size_t idx = 0; idx < sizeof(kTrueStrings) / sizeof(char*); idx++) {
        if (tjson_StringPiece_streq(token->spelling, kFalseStrings[idx])) {
          *value = 0;
          return 0;
        }
      }

      error->code = TJSON_WRONG_PRIMITIVE;
      error->loc = token->location;
      snprintf(error->msg, sizeof(error->msg),
               "Ambiguous truthiness of string '%.*s' can't be parsed as a "
               "boolean.",
               (int)tjson_StringPiece_size(token->spelling),
               token->spelling.begin);
      return -1;
    }

    default:
      break;
  }

  error->code = TJSON_WRONG_PRIMITIVE;
  error->loc = token->location;
  snprintf(error->msg, sizeof(error->msg),
           "Can't parse token of type %s as a boolean",
           tjson_TokenTypeNo_tostring(token->typeno));
  return -1;
}

int tjson_parse_value_string(const struct tjson_Token* token, char* buf,
                             size_t buflen, struct tjson_Error* error) {
  size_t off = 0;

  struct tjson_StringPiece substr =
      tjson_StringPiece_substr(token->spelling, 1, -1);
  for (const char* ptr = substr.begin; ptr != substr.end; ptr++) {
    buf[off++] = *ptr;
    if (off + 1 >= buflen) {
      error->code = TJSON_PARSE_OVERFLOW;
      error->loc = token->location;
      snprintf(error->msg, sizeof(error->msg),
               "Destination buffer too small (%d) for string %.*s", (int)buflen,
               (int)tjson_StringPiece_size(substr), substr.begin);
      buf[off] = '\0';
      return -1;
    }
  }
  buf[off] = '\0';
  return 0;
}

// -----------------------------------------------------------------------------
//    Value Parsers
// -----------------------------------------------------------------------------

int tjson_parse_uint64(tjson_ParseContext ctx, uint64_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_uint64(&event.token, value, ctx.error);
}

int tjson_parse_uint32(tjson_ParseContext ctx, uint32_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_uint32(&event.token, value, ctx.error);
}

int tjson_parse_uint16(tjson_ParseContext ctx, uint16_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_uint16(&event.token, value, ctx.error);
}

int tjson_parse_uint8(tjson_ParseContext ctx, uint8_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_uint8(&event.token, value, ctx.error);
}

int tjson_parse_int64(tjson_ParseContext ctx, int64_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_int64(&event.token, value, ctx.error);
}

int tjson_parse_int32(tjson_ParseContext ctx, int32_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_int32(&event.token, value, ctx.error);
}

int tjson_parse_int16(tjson_ParseContext ctx, int16_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_int16(&event.token, value, ctx.error);
}

int tjson_parse_int8(tjson_ParseContext ctx, int8_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_int8(&event.token, value, ctx.error);
}

int tjson_parse_double(tjson_ParseContext ctx, double* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_double(&event.token, value, ctx.error);
}

int tjson_parse_float(tjson_ParseContext ctx, float* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_float(&event.token, value, ctx.error);
}

int tjson_parse_boolean(tjson_ParseContext ctx, int8_t* value) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_boolean(&event.token, value, ctx.error);
}

int tjson_parse_string(tjson_ParseContext ctx, char* buf, size_t buflen) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }
  return tjson_parse_value_string(&event.token, buf, buflen, ctx.error);
}

// -----------------------------------------------------------------------------
//    Sink Functions
// -----------------------------------------------------------------------------

int tjson_sink_event(tjson_ParseContext ctx, const struct tjson_Event* event) {
  switch (event->typeno) {
    case TJSON_OBJECT_BEGIN:
      return tjson_sink_object(ctx, /*already_open=*/1);
    case TJSON_LIST_BEGIN:
      return tjson_sink_list(ctx, /*already_open=*/1);
    case TJSON_VALUE_LITERAL:
      return 0;
    default:
      ctx.error->code = TJSON_PARSE_UNEXPECTED_EVENT;
      ctx.error->loc = event->token.location;
      snprintf(ctx.error->msg, sizeof(ctx.error->msg),
               "%s:%d Unexpected %s (%d) event at %d:%d", __PRETTY_FUNCTION__,
               __LINE__, tjson_EventTypeNo_tostring(event->typeno),
               (int)event->typeno, (int)event->token.location.lineno,
               (int)event->token.location.colno);
      return -1;
  }
  return 1;
}

int tjson_sink_value(tjson_ParseContext ctx) {
  struct tjson_Event event;
  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }

  return tjson_sink_event(ctx, &event);
}

int tjson_sink_object(tjson_ParseContext ctx, int8_t already_open) {
  struct tjson_Event event;

  if (!already_open) {
    if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
      return -1;
    }

    if (event.typeno != TJSON_OBJECT_BEGIN) {
      ctx.error->code = TJSON_PARSE_UNEXPECTED_EVENT;
      ctx.error->loc = event.token.location;
      snprintf(ctx.error->msg, sizeof(ctx.error->msg),
               "%s:%d Unexpected %s (%d) event at %d:%d", __PRETTY_FUNCTION__,
               __LINE__, tjson_EventTypeNo_tostring(event.typeno),
               (int)event.typeno, (int)event.token.location.lineno,
               (int)event.token.location.colno);
      return 1;
    }
  }

  uint32_t object_count = 1;
  while (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error) == 0) {
    switch (event.typeno) {
      case TJSON_OBJECT_BEGIN:
        ++object_count;
        break;
      case TJSON_OBJECT_END:
        if (--object_count == 0) {
          return 0;
        }
        break;
      default:
        break;
    }
  }

  snprintf(ctx.error->msg, sizeof(ctx.error->msg),
           "Expected unreachable code %s:%d", __PRETTY_FUNCTION__, __LINE__);
  return -1;
}

int tjson_sink_list(tjson_ParseContext ctx, int8_t already_open) {
  struct tjson_Event event;

  if (!already_open) {
    if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
      return -1;
    }

    if (event.typeno != TJSON_LIST_BEGIN) {
      ctx.error->code = TJSON_PARSE_UNEXPECTED_EVENT;
      ctx.error->loc = event.token.location;
      snprintf(ctx.error->msg, sizeof(ctx.error->msg),
               "%s:%d Unexpected %s (%d) event at %d:%d", __PRETTY_FUNCTION__,
               __LINE__, tjson_EventTypeNo_tostring(event.typeno),
               (int)event.typeno, (int)event.token.location.lineno,
               (int)event.token.location.colno);
      return -1;
    }
  }

  uint32_t list_count = 1;
  while (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error) == 0) {
    switch (event.typeno) {
      case TJSON_LIST_BEGIN:
        ++list_count;
        break;
      case TJSON_LIST_END:
        if (--list_count == 0) {
          return 0;
        }
        break;
      default:
        break;
    }
  }

  snprintf(ctx.error->msg, sizeof(ctx.error->msg),
           "Expected unreachable code %s:%d", __PRETTY_FUNCTION__, __LINE__);
  return -1;
}

// -----------------------------------------------------------------------------
//    Aggregate parsers
// -----------------------------------------------------------------------------

int tjson_parse_object(tjson_ParseContext ctx,
                       int (*fielditem_callback)(void*, tjson_ParseContext,
                                                 tjson_StringPiece),
                       void* userdata) {
  struct tjson_Event event;

  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }

  if (event.typeno == TJSON_VALUE_LITERAL) {
    if (event.token.typeno == TJSON_NULL_LITERAL) {
      return 0;
    }
  }

  if (event.typeno != TJSON_OBJECT_BEGIN) {
    ctx.error->code = TJSON_PARSE_UNEXPECTED_EVENT;
    ctx.error->loc = event.token.location;
    snprintf(ctx.error->msg, sizeof(ctx.error->msg),
             "%s:%d (%s) Unexpected %s (%d) event at %d:%d", __FILE__, __LINE__,
             __PRETTY_FUNCTION__, tjson_EventTypeNo_tostring(event.typeno),
             (int)event.typeno, (int)event.token.location.lineno,
             (int)event.token.location.colno);
    return -1;
  }

  while (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error) == 0) {
    if (event.typeno == TJSON_OBJECT_END) {
      break;
    }

    if (event.typeno != TJSON_OBJECT_KEY) {
      ctx.error->code = TJSON_PARSE_UNEXPECTED_EVENT;
      ctx.error->loc = event.token.location;
      snprintf(
          ctx.error->msg, sizeof(ctx.error->msg),
          "%s:%d (%s) Unexpected %s (%d) event, wanted OBJECT_KEY at %d:%d",
          __FILE__, __LINE__, __PRETTY_FUNCTION__,
          tjson_EventTypeNo_tostring(event.typeno), (int)event.typeno,
          (int)event.token.location.lineno, (int)event.token.location.colno);
      return -1;
    }

    // strip quotes
    struct tjson_StringPiece key =
        tjson_StringPiece_substr(event.token.spelling, 1, -1);
    if (fielditem_callback(userdata, ctx, key)) {
      return -1;
    }
  }

  return 0;
}

int tjson_parse_list(tjson_ParseContext ctx,
                     int (*listitem_callback)(void*, tjson_ParseContext),
                     void* userdata) {
  struct tjson_Event event;

  if (tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error)) {
    return -1;
  }

  if (event.typeno != TJSON_LIST_BEGIN) {
    ctx.error->code = TJSON_PARSE_UNEXPECTED_EVENT;
    ctx.error->loc = event.token.location;
    snprintf(ctx.error->msg, sizeof(ctx.error->msg),
             "%s:%d Unexpected %s (%d) event at %d:%d", __PRETTY_FUNCTION__,
             __LINE__, tjson_EventTypeNo_tostring(event.typeno),
             (int)event.typeno, (int)event.token.location.lineno,
             (int)event.token.location.colno);
    return -1;
  }

  while (!tjson_LexerParser_peek_next_event(ctx.stream, &event, ctx.error)) {
    if (event.typeno == TJSON_LIST_END) {
      return tjson_LexerParser_get_next_event(ctx.stream, &event, ctx.error);
    }
    if (listitem_callback(userdata, ctx)) {
      return -1;
    }
  }
  return -1;
}

static int count_onlistitem(void* userdata, tjson_ParseContext ctx) {
  size_t* count = (size_t*)userdata;
  (*count) = (*count) + 1;
  return tjson_sink_value(ctx);
}

int tjson_count_list(tjson_ParseContext ctx, size_t* count) {
  // copy state
  tjson_LexerParser stream = *ctx.stream;
  ctx.stream = &stream;

  *count = 0;
  return tjson_parse_list(ctx, count_onlistitem, count);
}
