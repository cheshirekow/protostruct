#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <stdint.h>
#include <sys/types.h>

#if __cplusplus
extern "C" {
#endif

#define TJSON_VERSION \
  { 0, 1, 0, "dev", 0 }

// -----------------------------------------------------------------------------
//    StringPiece
// -----------------------------------------------------------------------------

typedef struct tjson_StringPiece {
  const char* begin;
  const char* end;
} tjson_StringPiece;

size_t tjson_StringPiece_size(struct tjson_StringPiece piece);

char tjson_StringPiece_get(struct tjson_StringPiece piece, size_t idx);

struct tjson_StringPiece tjson_StringPiece_substr(
    struct tjson_StringPiece source, size_t offset, ssize_t len);

int8_t tjson_StringPiece_streq(struct tjson_StringPiece piece,
                               const char* cstr);

int8_t tjson_StringPiece_startswith(struct tjson_StringPiece piece,
                                    const char* cstr);

int8_t tjson_StringPiece_endsswith(struct tjson_StringPiece piece,
                                   const char* cstr);

struct tjson_StringPiece tjson_StringPiece_fromstr(const char* cstr);

// Return a message digest (hash) of the string using the same hash function
// as tangent::hash() and tangent::runtime_hash()
uint64_t tjson_StringPiece_digest(tjson_StringPiece str);

// Return a message digest (hash) of the string using the same hash function
// as tangent::suci_hash() and tangent::runtime_suci_hash(). This is the
// "stripped-underscore, case-insensitive" version of tangent::hash().
uint64_t tjson_StringPiece_suci_digest(tjson_StringPiece str);

// -----------------------------------------------------------------------------
//    SourceLocation
// -----------------------------------------------------------------------------

// A reference to a location within the source string
typedef struct tjson_SourceLocation {
  uint32_t lineno;  // Number of newlines observed before this point
  uint32_t colno;   // Number of characters since the most recent newline
  uint32_t offset;  // Number of characters since the beginning
} tjson_SourceLocation;

// -----------------------------------------------------------------------------
//    Token
// -----------------------------------------------------------------------------

// Enumerates the possible token types
typedef enum tjson_TokenTypeNo {
  TJSON_STRING_LITERAL = 0,
  TJSON_NUMERIC_LITERAL,
  TJSON_BOOLEAN_LITERAL,
  TJSON_NULL_LITERAL,
  TJSON_WHITESPACE,
  TJSON_PUNCTUATION,
  TJSON_COMMENT,
  TJSON_INVALID_TOKEN,
} tjson_TokenTypeNo;

const char* tjson_TokenTypeNo_tostring(enum tjson_TokenTypeNo no);

// Fundamental syntatic unit of a JSON string. Scanner output is a list of
// these
typedef struct tjson_Token {
  enum tjson_TokenTypeNo typeno;
  struct tjson_StringPiece spelling;
  struct tjson_SourceLocation location;
} tjson_Token;

// -----------------------------------------------------------------------------
//    Event
// -----------------------------------------------------------------------------

// Enumerates parse events of interest
typedef enum tjson_EventTypeNo {
  TJSON_OBJECT_BEGIN,
  TJSON_OBJECT_KEY,
  TJSON_OBJECT_END,
  TJSON_LIST_BEGIN,
  TJSON_LIST_END,
  TJSON_VALUE_LITERAL,
  TJSON_INVALID,
} tjson_EventTypeNo;

const char* tjson_EventTypeNo_tostring(enum tjson_EventTypeNo value);

// Filled for each actionable event discovered by the parser. An actionable
// event is something like an object key, a value literal, or the start of an
// object/list value. Non-actionable events are things like whitespace or
// colon/comma punctuation.
typedef struct tjson_Event {
  enum tjson_EventTypeNo typeno;  //< What kind of event this is
  struct tjson_Token token;       //< the token for the event
} tjson_Event;

// -----------------------------------------------------------------------------
//    Error
// -----------------------------------------------------------------------------

typedef enum tjson_ErrorCode {
  TJSON_NOERROR = 0,
  TJSON_INTERNAL_ERROR,          //< bug in the code
  TJSON_LEX_INPUT_FINISHED,      //< lexer has no more input to read from
  TJSON_LEX_INVALID_TOKEN,       //< lexer encountered invalid json text
  TJSON_LEX_BAD_STATE,           //< lexer is in a bad state
  TJSON_PARSE_UNEXPECTED_TOKEN,  //< valid token but in the wrong place
  TJSON_PARSE_OOM,               //< Item-parser ran out of item storage
  TJSON_PARSE_BAD_STATE,         //< Parse failed previously
  TJSON_PARSE_UNEXPECTED_EVENT,  //< valid event, but not the one we wanted
  TJSON_PARSE_OVERFLOW,          //< would overflow
  TJSON_PARSE_SEMANTIC,          //< failed to parse a semantic value
  TJSON_WRONG_PRIMITIVE,         //< Attempt to parse from wrong primitive type
} tjson_ErrorCode;

const char* tjson_ErrorCode_tostring(enum tjson_ErrorCode value);

// Errors are reported via one of these objects
typedef struct tjson_Error {
  // Numeric identifier for the error
  enum tjson_ErrorCode code;

  // If the error occured during parse/emit and it was associated with a
  // particular location in the text, this will be set to that location.
  struct tjson_SourceLocation loc;

  // Will be filled with a description of the specific error
  char msg[512];
} tjson_Error;

// -----------------------------------------------------------------------------
//    Scanner
// -----------------------------------------------------------------------------

struct tjson_Scanner {
  // The content being parsed
  struct tjson_StringPiece _piece;

  // Stores the result of _init()
  int _init_state;

  // Total bytes required to store the contents of all numeric values assuming
  // that values are 64-bit aligned.
  uint64_t _numeric_storage;

  // Total bytes required to store the contents of all string values
  uint64_t _string_storage;

  union {
    uint32_t _options;
    struct {
      uint32_t _allow_comments : 1;
    };
  };

  // Current location of the input stream
  struct tjson_SourceLocation _loc;
};

// Initialize internal constants, etc.
// -1: Error occured
//  0: Successfully initialized
//  1: no-op, already successfully initialized
int tjson_Scanner_init(struct tjson_Scanner* scanner,
                       struct tjson_Error* error);

// Set the contents to be scanned.
int tjson_Scanner_begin(struct tjson_Scanner* scanner,
                        struct tjson_StringPiece content,
                        struct tjson_Error* error);

// Match and return the next token. Return 0 on success and -1 on error.
// if err is not NULL and an error occurs, will be set to a string
// describing the error message.
int tjson_Scanner_pump(struct tjson_Scanner* scanner, struct tjson_Token* tok,
                       struct tjson_Error* error);

// Match and return the next token. Return 0 on succes and -1 on error.
// if err is not NULL and an error occurs, will be set to a string
// describing the error message.
int tjson_Scanner_peek(struct tjson_Scanner* scanner, struct tjson_Token* tok,
                       struct tjson_Error* error);

// Scan/Tokenize the source string until completion; Store the tokens in `buf`.
// Return the number of tokens that were lexed (which may be greater than `n`)
// or -1 on error.
int tjson_lex(struct tjson_StringPiece source, struct tjson_Token* buf,
              uint64_t n, struct tjson_Error* error);

// Lex the entire source and return 0 if no errors are encountered. Return -1
// and fill `error` if any problems are encountered.
int tjson_verify_lex_piece(struct tjson_StringPiece source,
                           struct tjson_Error* error);

int tjson_verify_lex(const char* source, struct tjson_Error* error);

// -----------------------------------------------------------------------------
//    Parser
// -----------------------------------------------------------------------------

enum tjson_ParserState {
  TJSON_PARSING_VALUE = 0,    // Expect '{', '[' or a value literal
  TJSON_PARSING_LIST_OPEN,    // Expect a value or a closure
  TJSON_PARSING_OBJECT_OPEN,  // Expect a key or a closure
  TJSON_PARSING_KEY,          // Expect a string literal
  TJSON_PARSING_COLON,        // Expect a ':'
  TJSON_PARSING_CLOSURE,      // Expect a ']', '}', or ','
  TJSON_PARSING_ERROR,
};

// Manages the state machine for parsing JSON structure from a stream of
// tokens
typedef struct tjson_Parser {
  enum tjson_ParserState _state;
  enum tjson_EventTypeNo _group_stack[64];
  uint32_t _group_stack_size;
} tjson_Parser;

// Reset internal state
void tjson_Parser_reset(struct tjson_Parser* parser);

// Advance the internal parse state with the given token. Return 1 if an
// actionable event has occured.
/*
 * \return * 1 - an actionable event has occured. `event` is filled
 *         * 0 - no actionable event
 *         * -1 - an error has occured, `error` is filled if not null
 */
int tjson_Parser_handle_token(struct tjson_Parser* parser,
                              const struct tjson_Token* token,
                              struct tjson_Event* event,
                              struct tjson_Error* error, int8_t dry_run);

// -----------------------------------------------------------------------------
//    LexerParser
// -----------------------------------------------------------------------------

// A combined lexer/parser. Manages the incremental state of both
// simultaniously
typedef struct tjson_LexerParser {
  struct tjson_Scanner _scanner;
  struct tjson_Parser _parser;
  struct tjson_Token _token;
} tjson_LexerParser;

int tjson_LexerParser_init(struct tjson_LexerParser* lexerparser,
                           struct tjson_Error* error);

int tjson_LexerParser_begin(struct tjson_LexerParser* lexerparser,
                            struct tjson_StringPiece string,
                            struct tjson_Error* error);

// Consume tokens until the next semantic event. Return that event in `event`.
// Advance the token stream past the token that emitted that event.
int tjson_LexerParser_get_next_event(struct tjson_LexerParser* lexerparser,
                                     struct tjson_Event* event,
                                     struct tjson_Error* error);

// Consume tokens until the next semantic event. Return that event in `event`.
// Advance the token stream up to but not past the token that generated the
// `event`. The next call to `get_next_event` will return the same event.
int tjson_LexerParser_peek_next_event(struct tjson_LexerParser* lexerparser,
                                      struct tjson_Event* event,
                                      struct tjson_Error* error);

// Scan/Tokenize and Parse the source string until completion; Store the
// parser events in `buf`. Return the number of events that were parsed (which
// may be greater than `n`)  or -1 on error.
int tjson_parse(struct tjson_StringPiece source, struct tjson_Event* buf,
                uint32_t n, struct tjson_Error* error);

// Lex and Parse the entire source and return 0 if no errors are encountered.
// Return -1 and fill `error` if any problems are encountered.
int tjson_verify_parse_piece(struct tjson_StringPiece source,
                             struct tjson_Error* error);

// Lex and Parse the entire source and return 0 if no errors are encountered.
// Return -1 and fill `error` if any problems are encountered.
int tjson_verify_parse(const char* source, struct tjson_Error* error);

// -----------------------------------------------------------------------------
//    SerializeOpts
// -----------------------------------------------------------------------------

// Options for serialization
typedef struct tjson_SerializeOpts {
  uint32_t indent;        //< Number of spaces to use for indent
  char separators[2][3];  //< map and list separators, i.e. ":" and ","
} tjson_SerializeOpts;

// Default serialization option
extern const struct tjson_SerializeOpts tjson_DefaultOpts;

// Serialization options for a very compact JSON output
extern const struct tjson_SerializeOpts tjson_CompactOpts;

#if __cplusplus
}  // extern "C"
#endif
