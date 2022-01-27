// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>
#include "tangent/util/fixed_string_stream.h"

TEST(BaseStringStreamTest, StreamBuf) {
  char string[100] = "";
  util::FixedStreamBuf<char> streambuf{string};
  std::ostream stream{&streambuf};

  ASSERT_EQ(std::string(string), "");
  stream << "hello";
  ASSERT_EQ(std::string(string), "hello");
  stream << " world!";
  ASSERT_EQ(std::string(string), "hello world!");
  stream << "\nline two\nline three";
}

TEST(BaseStringStreamTest, StringStream) {
  util::FixedStringStream<100> stream;
  ASSERT_EQ(std::string(stream.str().data()), "");
  stream << "hello";
  ASSERT_EQ(std::string(stream.str().data()), "hello");
  stream << " world!";
  ASSERT_EQ(std::string(stream.str().data()), "hello world!");
  stream << "\nline two\nline three";

  std::string word;
  stream >> word;
  ASSERT_EQ(word, "hello");
  stream >> word;
  ASSERT_EQ(word, "world!");

  std::string line;
  std::getline(stream, line);
  ASSERT_EQ(line, "");
  std::getline(stream, line);
  ASSERT_EQ(line, "line two");
  std::getline(stream, line);
  ASSERT_EQ(line, "line three");

  stream.reset();
}
