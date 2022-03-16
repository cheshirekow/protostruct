// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <vector>

#include <gtest/gtest.h>

#include "tangent/tjson/tjson.h"

std::ostream& operator<<(std::ostream& out, tjson_StringPiece piece) {
  for (const char* ptr = piece.begin; ptr < piece.end; ptr++) {
    out << *ptr;
  }
  return out;
}

TEST(LexerTest, StringLiteralTest) {
  std::string test_string = "{\"foo\" : \"hello\"}";
  std::vector<tjson_Token> tokens;
  tokens.resize(10);

  tjson_Scanner scanner;
  tjson_Error err{TJSON_NOERROR, {}};
  tjson_StringPiece test_piece;
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());

  ASSERT_EQ(0, tjson_Scanner_init(&scanner, &err)) << err.msg;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 7; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << "at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[7], &err));
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_WHITESPACE, tokens[2].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[3].typeno);
  EXPECT_EQ(TJSON_WHITESPACE, tokens[4].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[5].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[6].typeno);
}

TEST(LexerTest, NumericLiteralTest) {
  std::vector<tjson_Token> tokens;
  tokens.resize(10);
  tjson_Error err{TJSON_NOERROR, {}};

  tjson_Scanner scanner;
  ASSERT_EQ(0, tjson_Scanner_init(&scanner, &err)) << err.msg;

  std::string test_string = "{\"foo\":1234}";

  tjson_StringPiece test_piece;
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":-1234}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":12.34}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":12.34e+10}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":12.34e-10}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);
}

TEST(LexerTest, BooleanLiteralTest) {
  std::vector<tjson_Token> tokens;
  tokens.resize(10);

  std::string test_string;
  tjson_StringPiece test_piece;
  tjson_Error err;
  tjson_Scanner scanner;

  ASSERT_EQ(0, tjson_Scanner_init(&scanner, &err)) << err.msg;

  test_string = "{\"foo\":true}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_BOOLEAN_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":false}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_BOOLEAN_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);
}

TEST(LexerTest, NullLiteralTest) {
  std::vector<tjson_Token> tokens;
  tokens.resize(10);

  std::string test_string;
  tjson_StringPiece test_piece;
  tjson_Error err;
  tjson_Scanner scanner;

  ASSERT_EQ(0, tjson_Scanner_init(&scanner, &err)) << err.msg;

  test_string = "{\"foo\":null}";
  test_piece.begin = &(*test_string.begin());
  test_piece.end = &(*test_string.end());
  err.code = TJSON_NOERROR;
  ASSERT_EQ(0, tjson_Scanner_begin(&scanner, test_piece, &err)) << err.msg;

  for (size_t idx = 0; idx < 5; ++idx) {
    tjson_StringPiece capture = scanner._piece;
    ASSERT_EQ(0, tjson_Scanner_pump(&scanner, &tokens[idx], &err))
        << err.msg << " for token " << idx << " at \""
        << tjson_StringPiece_substr(capture, 0, 10) << "\"";
  }

  ASSERT_EQ(-1, tjson_Scanner_pump(&scanner, &tokens[5], &err)) << err.msg;
  ASSERT_EQ(TJSON_LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(TJSON_STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(TJSON_NULL_LITERAL, tokens[3].typeno);
  EXPECT_EQ(TJSON_PUNCTUATION, tokens[4].typeno);
}

TEST(LexerTest, ErrorOnMalformed) {
  tjson_Error error;
  memset(&error, 0, sizeof(error));
  EXPECT_EQ(0, tjson_verify_lex("{\"foo\":\"bar\"}", &error))
      << "Error (" << error.code << "): " << error.msg;

  // Comments are not allowed
  ASSERT_GT(0,
            tjson_lex(tjson_StringPiece_fromstr("{} #hello"), NULL, 0, &error));
  ASSERT_GT(0, tjson_verify_lex("{} #hello", &error));
  EXPECT_EQ(TJSON_LEX_INVALID_TOKEN, error.code);
  ASSERT_GT(0, tjson_verify_lex("{} //hello", &error));
  EXPECT_EQ(TJSON_LEX_INVALID_TOKEN, error.code);

  // Incomplete string literal
  ASSERT_GT(0, tjson_verify_lex("{\"foo\" : \"hello", &error));
  EXPECT_EQ(TJSON_LEX_INVALID_TOKEN, error.code);

  // Invalid numeric literal
  // NOTE(josh): the string is valid up through
  // "{\n\"foo\" : 1,\n\"bar\": 12.3"
  ASSERT_GT(0, tjson_verify_lex("{\n\"foo\" : 1,\n\"bar\": 12.3x4}", &error));
  EXPECT_EQ(TJSON_LEX_INVALID_TOKEN, error.code);
  EXPECT_EQ(2, error.loc.lineno);
  EXPECT_EQ(11, error.loc.colno);
  EXPECT_EQ(24, error.loc.offset);
}
