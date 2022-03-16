// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/tjson/tjson.h"
#include "tangent/util/fallthrough.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
//    StringPiece
// -----------------------------------------------------------------------------

size_t tjson_StringPiece_size(struct tjson_StringPiece piece) {
  return piece.end - piece.begin;
}

char tjson_StringPiece_get(struct tjson_StringPiece piece, size_t idx) {
  return piece.begin[idx];
}

struct tjson_StringPiece tjson_StringPiece_substr(
    struct tjson_StringPiece source, size_t offset, ssize_t len) {
  struct tjson_StringPiece destination;
  if (offset > tjson_StringPiece_size(source)) {
    destination.begin = destination.end = source.end;
  }
  destination.begin = source.begin + offset;
  if (len < 0) {
    destination.end = source.end + len;
  } else {
    destination.end = destination.begin + len;
  }

  if (destination.begin > source.end) {
    destination.begin = source.end;
    destination.end = source.end;
  }
  if (destination.end > source.end) {
    destination.end = source.end;
  }

  return destination;
}

int8_t tjson_StringPiece_streq(struct tjson_StringPiece piece,
                               const char* cstr) {
  const char* piece_ptr = piece.begin;
  const char* cstr_ptr = cstr;

  while (piece_ptr < piece.end && *cstr_ptr != '\0' &&
         *piece_ptr == *cstr_ptr) {
    piece_ptr++;
    cstr_ptr++;
  }

  return (piece_ptr == piece.end && *cstr_ptr == '\0');
}

int8_t tjson_StringPiece_startswith(struct tjson_StringPiece piece,
                                    const char* cstr) {
  const char* piece_ptr = piece.begin;
  const char* cstr_ptr = cstr;

  while (piece_ptr < piece.end && *cstr_ptr != '\0' &&
         *piece_ptr == *cstr_ptr) {
    piece_ptr++;
    cstr_ptr++;
  }

  return (*cstr_ptr == '\0');
}

int8_t tjson_StringPiece_endswith(struct tjson_StringPiece piece,
                                  const char* cstr) {
  const char* piece_ptr = piece.end - 1;
  const char* cstr_ptr = cstr + strlen(cstr) - 1;

  while (piece_ptr >= piece.begin && cstr_ptr >= cstr &&
         *piece_ptr == *cstr_ptr) {
    piece_ptr--;
    cstr_ptr--;
  }

  return (cstr_ptr + 1 == cstr);
}

struct tjson_StringPiece tjson_StringPiece_fromstr(const char* cstr) {
  struct tjson_StringPiece out;
  out.begin = cstr;
  out.end = cstr + strlen(cstr);
  return out;
}

uint64_t tjson_StringPiece_digest(tjson_StringPiece str) {
  uint64_t digest = (str.end - str.begin);
  for (size_t idx = 0; str.begin + idx < str.end; ++idx) {
    digest = ((digest << 5) ^ (digest >> 27)) ^ str.begin[idx];
  }
  return digest;
}

uint64_t tjson_StringPiece_suci_digest(tjson_StringPiece str) {
  uint64_t digest = (str.end - str.begin);
  for (size_t idx = 0; str.begin + idx < str.end; ++idx) {
    if (str.begin[idx] == '_') {
      continue;
    }
    digest = ((digest << 5) ^ (digest >> 27)) ^ tolower(str.begin[idx]);
  }
  return digest;
}

// -----------------------------------------------------------------------------
//    Token
// -----------------------------------------------------------------------------

static const char* kTokenTypeToString[] = {
    "STRING_LITERAL",   //
    "NUMERIC_LITERAL",  //
    "BOOLEAN_LITERAL",  //
    "NULL_LITERAL",     //
    "WHITESPACE",       //
    "PUNCTUATION",      //
    "COMMENT",          //
};

const char* tjson_TokenTypeNo_tostring(enum tjson_TokenTypeNo no) {
  return kTokenTypeToString[no];
}

// -----------------------------------------------------------------------------
//    Error
// -----------------------------------------------------------------------------

const char* kErrorCodeToString[] = {
    "NOERROR",                 //
    "INTERNAL_ERROR",          //
    "LEX_INPUT_FINISHED",      //
    "LEX_INVALID_TOKEN",       //
    "LEX_BAD_STATE",           //
    "PARSE_UNEXPECTED_TOKEN",  //
    "PARSE_OOM",               //
    "PARSE_BAD_STATE",         //
    "PARSE_UNEXPECTED_EVENT",  //
    "PARSE_OVERFLOW",          //
    "PARSE_SEMANTIC",          //
    "WRONG_PRIMITIVE",         //
};

const char* tjson_ErrorCode_tostring(enum tjson_ErrorCode no) {
  return kErrorCodeToString[no];
}

// -----------------------------------------------------------------------------
//    Event
// -----------------------------------------------------------------------------

const char* kEventTypeNoToString[] = {
    "OBJECT_BEGIN",   //
    "OBJECT_KEY",     //
    "OBJECT_END",     //
    "LIST_BEGIN",     //
    "LIST_END",       //
    "VALUE_LITERAL",  //
    "INVALID",
};

const char* tjson_EventTypeNo_tostring(enum tjson_EventTypeNo no) {
  return kEventTypeNoToString[no];
}

// -----------------------------------------------------------------------------
//   Scanner
// -----------------------------------------------------------------------------

int tjson_Scanner_init(struct tjson_Scanner* scanner,
                       struct tjson_Error* error) {
  memset(scanner, 0, sizeof(struct tjson_Scanner));
  if (scanner->_init_state < 0) {
    return scanner->_init_state;
  }
  if (scanner->_init_state > 0) {
    return 0;
  }

  // NOTE(josh): we used to do some initialization here, but now we don't.

  scanner->_init_state = 1;
  return 0;
}

int tjson_Scanner_begin(struct tjson_Scanner* scanner,
                        struct tjson_StringPiece content,
                        struct tjson_Error* error) {
  scanner->_loc.lineno = 0;
  scanner->_loc.colno = 0;
  scanner->_loc.offset = 0;
  if (scanner->_init_state == 0) {
    tjson_Scanner_init(scanner, error);
  }
  if (scanner->_init_state < 0) {
    error->code = TJSON_LEX_BAD_STATE;
    error->loc = scanner->_loc;
    snprintf(error->msg, sizeof(error->msg), "Scanner is in a bad state: %d",
             (int)scanner->_init_state);
    return scanner->_init_state;
  }

  scanner->_numeric_storage = 0;
  scanner->_string_storage = 0;
  scanner->_piece = content;
  return 0;
}

static inline void advance_location(struct tjson_StringPiece str,
                                    struct tjson_SourceLocation* loc) {
  for (size_t idx = 0; idx < tjson_StringPiece_size(str); ++idx) {
    if (tjson_StringPiece_get(str, idx) == '\n') {
      loc->lineno++;
      loc->colno = 0;
    } else {
      loc->colno++;
    }
    loc->offset++;
  }
}

static inline int8_t is_punctuation(char c) {
  switch (c) {
    case ':':
    case ',':
    case '{':
    case '}':
    case '[':
    case ']':
      return 1;
    default:
      return 0;
  }
}

static inline int8_t is_whitespace(char c) {
  switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      return 1;
    default:
      return 0;
  }
}

static const char* consume_string_literal(struct tjson_StringPiece piece) {
  const char* ptr = piece.begin + 1;
  int8_t escape = 0;

  for (; ptr < piece.end; ptr++) {
    if (escape) {
      escape = 0;
      continue;
    }

    if (*ptr == '\\') {
      escape = 1;
      continue;
    }

    if (*ptr == '"') {
      return ptr + 1;
    }
  }

  return ptr;
}

static void consume_token(struct tjson_StringPiece piece,
                          struct tjson_Token* tok) {
  // TJSON_PUNCTUATION
  if (is_punctuation(tjson_StringPiece_get(piece, 0))) {
    tok->typeno = TJSON_PUNCTUATION;
    tok->spelling.begin = piece.begin;
    tok->spelling.end = piece.begin + 1;
    tok->typeno = TJSON_PUNCTUATION;
    return;
  }

  // TJSON_WHITESPACE
  if (is_whitespace(tjson_StringPiece_get(piece, 0))) {
    const char* ptr = piece.begin;
    while (ptr < piece.end && is_whitespace(*ptr)) {
      ptr++;
    }

    tok->spelling.begin = piece.begin;
    tok->spelling.end = ptr;
    tok->typeno = TJSON_WHITESPACE;
    return;
  }

  // TJSON_COMMENT
  if (tjson_StringPiece_startswith(piece, "//") ||
      tjson_StringPiece_startswith(piece, "#")) {
    // Consume up to the next newline character (this is a line comment)
    const char* ptr = piece.begin;
    while (ptr < piece.end && *ptr != '\n') {
      ptr++;
    }

    tok->spelling.begin = piece.begin;
    tok->spelling.end = ptr;
    tok->typeno = TJSON_COMMENT;
    return;
  }

  // TJSON_BOOLEAN_LITERAL
  if (tjson_StringPiece_startswith(piece, "true")) {
    tok->spelling.begin = piece.begin;
    tok->spelling.end = piece.begin + strlen("true");
    tok->typeno = TJSON_BOOLEAN_LITERAL;
    return;
  }

  if (tjson_StringPiece_startswith(piece, "false")) {
    tok->spelling.begin = piece.begin;
    tok->spelling.end = piece.begin + strlen("false");
    tok->typeno = TJSON_BOOLEAN_LITERAL;
    return;
  }

  // TJSON_NULL LITERAL
  if (tjson_StringPiece_startswith(piece, "null")) {
    tok->spelling.begin = piece.begin;
    tok->spelling.end = piece.begin + strlen("null");
    tok->typeno = TJSON_NULL_LITERAL;
    return;
  }

  // TJSON_STRING_LITERAL
  if (tjson_StringPiece_startswith(piece, "\"")) {
    tok->spelling.begin = piece.begin;
    tok->spelling.end = consume_string_literal(piece);
    tok->typeno = TJSON_STRING_LITERAL;
    return;
  }

  // TJSON_NUMERIC_LITERAL
  tok->spelling.begin = piece.begin;
  double value = strtod(piece.begin, (char**)&tok->spelling.end);
  tok->typeno = TJSON_NUMERIC_LITERAL;
  if (value == 0 && tjson_StringPiece_size(tok->spelling) == 0) {
    tok->typeno = TJSON_INVALID_TOKEN;
    return;
  }
  return;
}

int tjson_Scanner_pump_impl(struct tjson_Scanner* scanner,
                            struct tjson_Token* tok, struct tjson_Error* error,
                            int8_t peek) {
  if (tjson_StringPiece_size(scanner->_piece) < 1) {
    error->code = TJSON_LEX_INPUT_FINISHED;
    error->loc = scanner->_loc;
    snprintf(error->msg, sizeof(error->msg),
             "The input stream is empty. Either parsing is finished or the "
             "data is truncated.");
    return -1;
  }

  consume_token(scanner->_piece, tok);
  if (tok->typeno == TJSON_STRING_LITERAL &&
      !tjson_StringPiece_endswith(tok->spelling, "\"")) {
    tok->typeno = TJSON_INVALID_TOKEN;
  }

  if (tok->typeno == TJSON_COMMENT && !scanner->_allow_comments) {
    tok->typeno = TJSON_INVALID_TOKEN;
  }

  if (tok->typeno == TJSON_INVALID_TOKEN) {
    error->code = TJSON_LEX_INVALID_TOKEN;
    error->loc = scanner->_loc;
    snprintf(
        error->msg, sizeof(error->msg),
        "An invalid input token was encountered. Source is not valid json. "
        "At %d:%d",
        (int)scanner->_loc.lineno, (int)scanner->_loc.colno);
    return -1;
  }

  tok->location = scanner->_loc;
  if (peek) {
    return 0;
  }

  advance_location(tok->spelling, &(scanner->_loc));
  scanner->_piece.begin = tok->spelling.end;

  switch (tok->typeno) {
    case TJSON_NUMERIC_LITERAL:
      scanner->_numeric_storage += sizeof(uint32_t);
      break;
    case TJSON_STRING_LITERAL:
      scanner->_string_storage += tjson_StringPiece_size(tok->spelling) + 1;
      break;
    default:
      break;
  }

  return 0;
}

int tjson_Scanner_pump(struct tjson_Scanner* scanner, struct tjson_Token* tok,
                       struct tjson_Error* error) {
  return tjson_Scanner_pump_impl(scanner, tok, error, /*peek=*/0);
}

int tjson_Scanner_peek(struct tjson_Scanner* scanner, struct tjson_Token* tok,
                       struct tjson_Error* error) {
  return tjson_Scanner_pump_impl(scanner, tok, error, /*peek=*/1);
}

// -----------------------------------------------------------------------------
//   High Level Lex Functions
// -----------------------------------------------------------------------------

int tjson_lex(struct tjson_StringPiece source, struct tjson_Token* buf,
              uint64_t n, struct tjson_Error* error) {
  struct tjson_Scanner scanner;
  if (tjson_Scanner_init(&scanner, error)) {
    return -1;
  }

  if (tjson_Scanner_begin(&scanner, source, error)) {
    return -2;
  }

  struct tjson_Error local_error;
  if (!error) {
    error = &local_error;
  }

  size_t ntokens = 0;
  for (; ntokens < n; ++ntokens) {
    if (tjson_Scanner_pump(&scanner, &(buf[ntokens]), error) < 0) {
      if (error->code == TJSON_LEX_INPUT_FINISHED) {
        return ntokens;
      } else {
        return -3;
      }
    }
  }

  struct tjson_Token local_token;
  while (tjson_Scanner_pump(&scanner, &local_token, error) >= 0) {
    ntokens++;
  }

  if (error->code == TJSON_LEX_INPUT_FINISHED) {
    return ntokens;
  } else {
    return -3;
  }
}

int tjson_verify_lex_piece(struct tjson_StringPiece source,
                           struct tjson_Error* error) {
  int result = tjson_lex(source, NULL, 0, error);
  if (result < 0) {
    return result;
  } else {
    return 0;
  }
}

int tjson_verify_lex(const char* source, struct tjson_Error* error) {
  return tjson_verify_lex_piece(tjson_StringPiece_fromstr(source), error);
}

// -----------------------------------------------------------------------------
//    Parser
// -----------------------------------------------------------------------------

void tjson_Parser_reset(struct tjson_Parser* parser) {
  parser->_state = TJSON_PARSING_VALUE;
  parser->_group_stack_size = 0;
}

static enum tjson_EventTypeNo groupstack_back(struct tjson_Parser* parser) {
  return parser->_group_stack[parser->_group_stack_size - 1];
}

static void groupstack_push(struct tjson_Parser* parser,
                            enum tjson_EventTypeNo event_typeno) {
  parser->_group_stack[parser->_group_stack_size] = event_typeno;
  parser->_group_stack_size++;
}

static int groupstack_check_overflow(struct tjson_Parser* parser,
                                     struct tjson_SourceLocation loc,
                                     struct tjson_Error* error) {
  if (parser->_group_stack_size + 1 >
      sizeof(parser->_group_stack) / sizeof(parser->_group_stack[0])) {
    error->code = TJSON_INTERNAL_ERROR;
    error->loc = loc;
    snprintf(error->msg, sizeof(error->msg), "Group stack overflow at %d:%d",
             (int)loc.lineno, (int)loc.colno);
    return -1;
  }
  return 0;
}

static void groupstack_pop(struct tjson_Parser* parser) {
  parser->_group_stack_size--;
}

int tjson_Parser_handle_token(struct tjson_Parser* parser,
                              const struct tjson_Token* token,
                              struct tjson_Event* event,
                              struct tjson_Error* error, int8_t dry_run) {
  event->token = *token;
  if (token->typeno == TJSON_WHITESPACE || token->typeno == TJSON_COMMENT) {
    return 0;
  }

  switch (parser->_state) {
    case TJSON_PARSING_LIST_OPEN: {
      if (token->typeno == TJSON_PUNCTUATION &&
          (parser->_group_stack_size > 0) &&
          groupstack_back(parser) == TJSON_LIST_BEGIN &&
          tjson_StringPiece_streq(token->spelling, "]")) {
        event->typeno = TJSON_LIST_END;
        if (dry_run) {
          return 1;
        }
        groupstack_pop(parser);
        parser->_state = TJSON_PARSING_CLOSURE;
        return 1;
      }
      TANGENT_FALLTHROUGH
    }

    case TJSON_PARSING_VALUE: {
      if (token->typeno == TJSON_PUNCTUATION) {
        if (!tjson_StringPiece_streq(token->spelling, "{") &&
            !tjson_StringPiece_streq(token->spelling, "[")) {
          error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
          error->loc = token->location;
          snprintf(error->msg, sizeof(error->msg),
                   "Expected '{' or '[' but got %.*s",
                   (int)tjson_StringPiece_size(token->spelling),
                   token->spelling.begin);
          return -1;
        }

        if (tjson_StringPiece_streq(token->spelling, "{")) {
          event->typeno = TJSON_OBJECT_BEGIN;
          if (dry_run) {
            return 1;
          }
          if (groupstack_check_overflow(parser, token->location, error)) {
            return -1;
          }
          groupstack_push(parser, TJSON_OBJECT_BEGIN);
          parser->_state = TJSON_PARSING_OBJECT_OPEN;
        } else {
          event->typeno = TJSON_LIST_BEGIN;
          if (dry_run) {
            return 1;
          }
          if (groupstack_check_overflow(parser, token->location, error)) {
            return -1;
          }
          groupstack_push(parser, TJSON_LIST_BEGIN);
          parser->_state = TJSON_PARSING_LIST_OPEN;
        }
        return 1;
      } else {
        event->typeno = TJSON_VALUE_LITERAL;
        if (dry_run) {
          return 1;
        }
        parser->_state = TJSON_PARSING_CLOSURE;
        return 1;
      }
      break;
    }

    case TJSON_PARSING_OBJECT_OPEN: {
      if (token->typeno == TJSON_PUNCTUATION &&
          (parser->_group_stack_size > 0) &&
          groupstack_back(parser) == TJSON_OBJECT_BEGIN &&
          tjson_StringPiece_streq(token->spelling, "}")) {
        event->typeno = TJSON_OBJECT_END;
        if (dry_run) {
          return 1;
        }
        groupstack_pop(parser);
        parser->_state = TJSON_PARSING_CLOSURE;
        return 1;
      }
      TANGENT_FALLTHROUGH
    }

    case TJSON_PARSING_KEY: {
      if (token->typeno != TJSON_STRING_LITERAL) {
        error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
        error->loc = token->location;
        snprintf(error->msg, sizeof(error->msg),
                 "Expected a string literal (key) but got %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);
        return -1;
      }

      if (parser->_group_stack_size < 1) {
        error->code = TJSON_INTERNAL_ERROR;
        error->loc = token->location;
        snprintf(error->msg, sizeof(error->msg), "_group_stack is empty: %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);
        return -1;
      }

      if (groupstack_back(parser) != TJSON_OBJECT_BEGIN) {
        error->code = TJSON_INTERNAL_ERROR;
        error->loc = token->location;
        snprintf(error->msg, sizeof(error->msg),
                 "_group_stack is not an object: %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);

        return -1;
      }

      event->typeno = TJSON_OBJECT_KEY;
      if (dry_run) {
        return 1;
      }

      parser->_state = TJSON_PARSING_COLON;
      return 1;
    }

    case TJSON_PARSING_COLON: {
      if (token->typeno != TJSON_PUNCTUATION ||
          !tjson_StringPiece_streq(token->spelling, ":")) {
        error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
        error->loc = token->location;

        snprintf(error->msg, sizeof(error->msg),
                 "Expected a colon (':') but got: %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);
        return -1;
      }
      if (dry_run) {
        return 0;
      }

      parser->_state = TJSON_PARSING_VALUE;
      return 0;
    }

    case TJSON_PARSING_CLOSURE: {
      if (token->typeno != TJSON_PUNCTUATION) {
        error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
        error->loc = token->location;
        snprintf(error->msg, sizeof(error->msg),
                 "Expected ']', '}', or ',' but got: %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);
        return -1;
      }

      if (parser->_group_stack_size < 1) {
        error->code = TJSON_INTERNAL_ERROR;
        error->loc = token->location;
        snprintf(error->msg, sizeof(error->msg), "group_stack_ is empty: %.*s",
                 (int)tjson_StringPiece_size(token->spelling),
                 token->spelling.begin);
        return -1;
      }

      if (tjson_StringPiece_streq(token->spelling, ",")) {
        if (groupstack_back(parser) == TJSON_LIST_BEGIN) {
          if (dry_run) {
            return 0;
          }

          parser->_state = TJSON_PARSING_VALUE;
          return 0;
        } else if (groupstack_back(parser) == TJSON_OBJECT_BEGIN) {
          if (dry_run) {
            return 0;
          }
          parser->_state = TJSON_PARSING_KEY;
          return 0;
        } else {
          error->code = TJSON_INTERNAL_ERROR;
          error->loc = token->location;
          snprintf(error->msg, sizeof(error->msg),
                   "Top of group stack is not a list or object: %.*s",
                   (int)tjson_StringPiece_size(token->spelling),
                   token->spelling.begin);
          return -1;
        }
      }

      if (groupstack_back(parser) == TJSON_LIST_BEGIN) {
        if (!tjson_StringPiece_streq(token->spelling, "]")) {
          error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
          error->loc = token->location;
          snprintf(error->msg, sizeof(error->msg), "Expected ']' but got: %.*s",
                   (int)tjson_StringPiece_size(token->spelling),
                   token->spelling.begin);
          return -1;
        }

        event->typeno = TJSON_LIST_END;
        if (dry_run) {
          return 1;
        }
        groupstack_pop(parser);
        parser->_state = TJSON_PARSING_CLOSURE;
        return 1;
      }

      if (groupstack_back(parser) == TJSON_OBJECT_BEGIN) {
        if (!tjson_StringPiece_streq(token->spelling, "}")) {
          error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
          error->loc = token->location;
          snprintf(error->msg, sizeof(error->msg),
                   "Expected '}' but got: '%.*s'",
                   (int)tjson_StringPiece_size(token->spelling),
                   token->spelling.begin);
          return -1;
        }

        event->typeno = TJSON_OBJECT_END;
        if (dry_run) {
          return 1;
        }
        groupstack_pop(parser);
        return 1;
      }

      error->code = TJSON_PARSE_UNEXPECTED_TOKEN;
      error->loc = token->location;
      snprintf(error->msg, sizeof(error->msg),
               "Expected ']', '}', or ',' but got: %.*s",
               (int)tjson_StringPiece_size(token->spelling),
               token->spelling.begin);
      return -1;
    }

    case TJSON_PARSING_ERROR: {
      error->code = TJSON_PARSE_BAD_STATE;
      error->loc = token->location;
      snprintf(error->msg, sizeof(error->msg), "Parser is in an error state");
      return -1;
    }

    default: {
      error->code = TJSON_INTERNAL_ERROR;
      error->loc = token->location;
      snprintf(error->msg, sizeof(error->msg), "Unknown parser state: %d",
               (int)parser->_state);
      return -1;
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
//    LexerParser
// -----------------------------------------------------------------------------

int tjson_LexerParser_init(struct tjson_LexerParser* lexerparser,
                           struct tjson_Error* error) {
  return tjson_Scanner_init(&(lexerparser->_scanner), error);
}

int tjson_LexerParser_begin(struct tjson_LexerParser* lexerparser,
                            struct tjson_StringPiece string,
                            struct tjson_Error* error) {
  tjson_Parser_reset(&lexerparser->_parser);
  return tjson_Scanner_begin(&lexerparser->_scanner, string, error);
}

int tjson_LexerParser_get_next_event(struct tjson_LexerParser* lexerparser,
                                     struct tjson_Event* event,
                                     struct tjson_Error* error) {
  int result = 0;
  while (result == 0) {
    result =
        tjson_Scanner_pump(&lexerparser->_scanner, &lexerparser->_token, error);
    if (result < 0) {
      return result;
    }

    result =
        tjson_Parser_handle_token(&lexerparser->_parser, &lexerparser->_token,
                                  event, error, /*dry_run=*/0);
    if (result < 0) {
      return result;
    }
    if (result > 0) {
      return 0;
    }
    // If result == 0 then the token did not instigate an event
  }
  return result;
}

int tjson_LexerParser_peek_next_event(struct tjson_LexerParser* lexerparser,
                                      struct tjson_Event* event,
                                      struct tjson_Error* error) {
  int result = 0;
  while (result == 0) {
    result =
        tjson_Scanner_peek(&lexerparser->_scanner, &lexerparser->_token, error);
    if (result < 0) {
      return result;
    }

    result =
        tjson_Parser_handle_token(&lexerparser->_parser, &lexerparser->_token,
                                  event, error, /*dry_run=*/1);
    if (result < 0) {
      return result;
    }
    if (result > 0) {
      return 0;
    }

    // If result == 0 then the token did not instigate an event. Since it was
    // non-event producing, we can advance the stream.
    result =
        tjson_Scanner_pump(&lexerparser->_scanner, &lexerparser->_token, error);
    tjson_Parser_handle_token(&lexerparser->_parser, &lexerparser->_token,
                              event, error, /*dry_run=*/0);
  }
  return 0;
}

// -----------------------------------------------------------------------------
//    High Level Parse Functinos
// -----------------------------------------------------------------------------

int tjson_parse(struct tjson_StringPiece source, struct tjson_Event* buf,
                uint32_t n, struct tjson_Error* error) {
  struct tjson_LexerParser parser;

  if (tjson_LexerParser_init(&parser, error)) {
    return -1;
  }
  if (tjson_LexerParser_begin(&parser, source, error)) {
    return -2;
  }

  struct tjson_Error local_error;
  if (!error) {
    error = &local_error;
  }

  size_t nevents = 0;
  for (; nevents < n; ++nevents) {
    int result =
        tjson_LexerParser_get_next_event(&parser, &buf[nevents], error);
    if (result < 0) {
      if (error->code == TJSON_LEX_INPUT_FINISHED) {
        return nevents;
      } else {
        return -3;
      }
    }
  }

  struct tjson_Event local_event;
  for (; 1; ++nevents) {
    int result = tjson_LexerParser_get_next_event(&parser, &local_event, error);
    if (result < 0) {
      if (error->code == TJSON_LEX_INPUT_FINISHED) {
        return nevents;
      } else {
        return -5;
      }
    }
  }
}

int tjson_verify_parse_piece(struct tjson_StringPiece source,
                             struct tjson_Error* error) {
  if (tjson_parse(source, NULL, 0, error) >= 0) {
    return 0;
  } else {
    return -1;
  }
}

int tjson_verify_parse(const char* source, struct tjson_Error* error) {
  return tjson_verify_parse_piece(tjson_StringPiece_fromstr(source), error);
}

const struct tjson_SerializeOpts tjson_DefaultOpts = {
    .indent = 2,
    .separators = {": ", ","},
};

const struct tjson_SerializeOpts tjson_jCompactOpts = {
    .indent = 0, .separators = {":", ","}};
