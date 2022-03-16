// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

TEST(KeywordTest, TestProofOfConcept) {
  using namespace argue::keywords;  // NOLINT

  argue::Parser::Metadata meta{true};
  argue::Parser parser{meta};

  std::string foo;
  int bar;
  // clang-format off
  parser.add_argument("-f", "--foo", action="store",dest=&foo);  // NOLINT
  parser.add_argument("-b", "--bar", dest=&bar);  // NOLINT
  // clang-format on

  std::stringstream logstream{};

  std::vector<std::string> args = {"progname", "--foo", "hello", "--bar",
                                   "1234"};
  char* argv[5] = {&args[0][0], &args[1][0], &args[2][0], &args[3][0],
                   &args[4][0]};
  int argc = 5;
  int result = parser.parse_args(argc, static_cast<char**>(argv), &logstream);
  ASSERT_EQ(result, argue::PARSE_FINISHED) << logstream.str();
  EXPECT_EQ(foo, "hello");
  EXPECT_EQ(bar, 1234);
}
