// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "tangent/util/exception.h"

TEST(ExceptionTests, TestStreams) {
  try {
    TANGENT_ASSERT(true) << "Hello";
  } catch (tangent::Exception& ex) {
    FAIL() << "Assertion should have been true";
  }

  tangent::Exception ex_copy{"", {}};
  try {
    TANGENT_ASSERT(false) << "Hello world!";
  } catch (tangent::Exception& ex) {
    ex_copy = ex;
  }
  EXPECT_EQ(ex_copy.message, "Hello world!");

  try {
    TANGENT_THROW() << "Hello two!";
  } catch (tangent::Exception& ex) {
    ex_copy = ex;
  }
  EXPECT_EQ(ex_copy.message, "Hello two!");
}
