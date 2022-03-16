// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>
#include "tangent/util/array_stack.h"

struct Foo {
  int x;
  int y;

  Foo(int x, int y) : x{x}, y{y} {}
};

TEST(ArrayStackTest, BasicTest) {
  tangent::ArrayStack<Foo, 10> stack{};
  ASSERT_EQ(stack.capacity(), 10);
  ASSERT_EQ(stack.size(), 0);
  stack.emplace_back(1, 2);
  stack.emplace_back(3, 4);
  ASSERT_EQ(stack.size(), 2);
  ASSERT_EQ(stack[0].x, 1);
  ASSERT_EQ(stack[0].y, 2);
  ASSERT_EQ(stack[1].x, 3);
  ASSERT_EQ(stack[1].y, 4);
  stack.push_back(Foo{5, 6});
  ASSERT_EQ(stack.size(), 3);
  ASSERT_EQ(stack[2].x, 5);
  ASSERT_EQ(stack[2].y, 6);
}
