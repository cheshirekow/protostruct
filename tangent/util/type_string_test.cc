// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "tangent/util/type_string.h"

template <typename T>
struct Foo {};

TEST(TypeStringTest, TestSomeNames) {
  std::string actual = hacky_type_string<uint16_t>();

  EXPECT_TRUE(actual == "unsigned short" || actual == "short unsigned int")
      << "actual = " << actual;
  EXPECT_EQ("uint8_t", type_string<uint8_t>());
  EXPECT_EQ("uint16_t", type_string<uint16_t>());
  EXPECT_EQ("std::string", type_string<std::string>());
  // EXPECT_EQ("Foo<std::__cxx11::basic_string<char> >",
  // type_string<Foo<std::string>>()); EXPECT_EQ("char[]",
  // type_string<char[]>());
}
