// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

TEST(string_to_nargsTest, CorrectlyParsesExampleQueries) {
  EXPECT_EQ(argue::INVALID_NARGS, argue::string_to_nargs("!"));
  EXPECT_EQ(argue::ONE_OR_MORE, argue::string_to_nargs("+"));
  EXPECT_EQ(argue::ZERO_OR_MORE, argue::string_to_nargs("*"));
  EXPECT_EQ(argue::ZERO_OR_ONE, argue::string_to_nargs("?"));
}

TEST(ArgTypeTest, CorrectlyParsesExampleQueries) {
  EXPECT_EQ(argue::SHORT_FLAG, argue::get_arg_type("-f"));
  EXPECT_EQ(argue::LONG_FLAG, argue::get_arg_type("--foo"));
  EXPECT_EQ(argue::POSITIONAL, argue::get_arg_type("foo"));
}
