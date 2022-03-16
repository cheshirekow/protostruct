// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

// NOTE(josh): test templates will overflow when instanciated, but codepath is
// disabled in runtime.
#pragma GCC diagnostic ignored "-Woverflow"
#if __clang__
#pragma clang diagnostic ignored "-Wconstant-conversion"
#endif

#define EXPECT_PARSE(QUERY, VALUE) this->ExpectParse(QUERY, VALUE, __LINE__)

template <typename T>
class ParseTest : public ::testing::Test {
 protected:
  void ExpectParse(const std::string& query, const T expect_value,
                   int line_no) {
    T value = 0;
    argue::parse(query, &value);
    EXPECT_EQ(expect_value, value) << __FILE__ << ":" << line_no;
  }
};

template <typename T>
class UnsignedParseTest : public ParseTest<T> {};
typedef ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t> UnsignedTypes;
TYPED_TEST_CASE(UnsignedParseTest, UnsignedTypes);
TYPED_TEST(UnsignedParseTest, ParsesExamples) {
  EXPECT_PARSE("0", 0);
  EXPECT_PARSE("1", 1);
  EXPECT_PARSE("10", 10);
  EXPECT_PARSE("100", 100);
  EXPECT_PARSE("123", 123);

  if (sizeof(TypeParam) > 0) {
    EXPECT_PARSE("255", 255);
  }

  if (sizeof(TypeParam) > 1) {
    EXPECT_PARSE("65535", 65535);
  }

  if (sizeof(TypeParam) > 2) {
    EXPECT_PARSE("4294967295", 4294967295L);
  }

  if (sizeof(TypeParam) > 4) {
    EXPECT_PARSE("18446744073709551615", 18446744073709551615UL);
  }
}

template <typename T>
class SignedParseTest : public ParseTest<T> {};
typedef ::testing::Types<int8_t, int16_t, int32_t, int64_t> SignedTypes;
TYPED_TEST_CASE(SignedParseTest, SignedTypes);
TYPED_TEST(SignedParseTest, ParsesExamples) {
  EXPECT_PARSE("0", 0);
  EXPECT_PARSE("1", 1);
  EXPECT_PARSE("-1", -1);
  EXPECT_PARSE("10", 10);
  EXPECT_PARSE("-10", -10);
  EXPECT_PARSE("100", 100);
  EXPECT_PARSE("-100", -100);
  EXPECT_PARSE("123", 123);
  EXPECT_PARSE("-123", -123);

  if (sizeof(TypeParam) > 0) {
    EXPECT_PARSE("128", 128);
    EXPECT_PARSE("-127", -127);
  }

  if (sizeof(TypeParam) > 1) {
    EXPECT_PARSE("32768", 32768);
    EXPECT_PARSE("-32767", -32767);
  }

  if (sizeof(TypeParam) > 2) {
    EXPECT_PARSE("2147483648", 2147483648);
    EXPECT_PARSE("-2147483647", -2147483647);
  }

  if (sizeof(TypeParam) > 4) {
    EXPECT_PARSE("9223372036854775808", 9223372036854775808UL);
    EXPECT_PARSE("-9223372036854775807", -9223372036854775807LL);
  }
}

template <typename T>
class FloatParseTest : public ParseTest<T> {};
typedef ::testing::Types<float, double> FloatTypes;
TYPED_TEST_CASE(FloatParseTest, FloatTypes);
TYPED_TEST(FloatParseTest, ParsesExamples) {
  EXPECT_PARSE("0", 0);
  EXPECT_PARSE("1", 1);
  EXPECT_PARSE("-1", -1);
  EXPECT_PARSE("10", 10);
  EXPECT_PARSE("-10", -10);
  EXPECT_PARSE("100", 100);
  EXPECT_PARSE("-100", -100);
  EXPECT_PARSE("12.345", 12.345);
  EXPECT_PARSE("-12.345", -12.345);
  EXPECT_PARSE("987654.321", 987654.321);
  EXPECT_PARSE("-987654.321", -987654.321);
}
