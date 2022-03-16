// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <array>
#include <string>

#include <gtest/gtest.h>

#include "tangent/tjson/tjson.h"

TEST(ParserTest, Test_streq) {
  const char* teststr = "hello world";
  ASSERT_TRUE(
      tjson_StringPiece_streq(tjson_StringPiece_fromstr(teststr), teststr));
}
