// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

TEST(AssertTest, TypeTagsThrowCorrectExceptionType) {
  bool there_was_an_exception = false;

  try {
    ARGUE_ASSERT(BUG, true) << "Hello!";
  } catch (...) {
    there_was_an_exception = true;
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_FALSE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(BUG, false, "Hello World");
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ(ex.typeno, argue::Exception::BUG);
    EXPECT_EQ("Hello World", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello World");
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World", ex.message);
    EXPECT_EQ(ex.typeno, argue::Exception::CONFIG_ERROR);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(INPUT_ERROR, false, "Hello World");
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World", ex.message);
    EXPECT_EQ(ex.typeno, argue::Exception::INPUT_ERROR);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);
}

TEST(AssertTest, AllMessageMechanismsWork) {
  bool there_was_an_exception = false;

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello World:42");
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
    EXPECT_EQ(ex.typeno, argue::Exception::CONFIG_ERROR);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false)
        << fmt::format("Hello {}:{}", "World", 42);
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
    EXPECT_EQ(ex.typeno, argue::Exception::CONFIG_ERROR);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello") << " World:" << 42;
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
    EXPECT_EQ(ex.typeno, argue::Exception::CONFIG_ERROR);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false) << "Hello World:" << 42;
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
    EXPECT_EQ(ex.typeno, argue::Exception::CONFIG_ERROR);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);
}

void Baz() {
  ARGUE_ASSERT(BUG, false) << "Hello!";
}

void Bar() {
  Baz();
}

void Foo() {
  Bar();
}

TEST(AssertTest, BugHasStackTrace) {
  bool there_was_an_exception = false;
  try {
    Foo();
  } catch (const argue::Exception& ex) {
    there_was_an_exception = true;
    EXPECT_EQ(ex.typeno, argue::Exception::BUG);
    EXPECT_EQ("Hello!", ex.message);
    ASSERT_LT(3, ex.stack_trace.size());
    // NOTE(josh): problematic as stack gets optimized out :o
    if (!ex.stack_trace[0].name.empty()) {
      EXPECT_EQ("Baz()", ex.stack_trace[0].name);
      // TODO(josh): this randomly started failing during buildbot builds
      // on 01/30/2019 which doesn't make much since because we're running
      // in a container. What's double weird is that the same test continues
      // to pass on my workstation and laptop. In buildbot, instead of holding
      // the expected values, these two strings are:
      //
      //  AssertTest_BugHasStackTrace_Test::TestBody()
      //  void testing::internal::HandleExceptionsInMethodIfSupported...
      //
      // EXPECT_EQ("Bar()", ex.stack_trace[1].name);
      // EXPECT_EQ("Foo()", ex.stack_trace[2].name);
    }
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);
}

bool ReturnsTrue(int a, int b, int c) {
  return true;
}

template <typename T1, typename T2, typename T3>
bool TReturnsTrue(T1 a, T2 b, T3 c) {
  return true;
}

TEST(AssertTest, MacroTest) {
  ARGUE_ASSERT(BUG, (1 < 2), "1 >= 2??");
  ARGUE_ASSERT(BUG, (1 < 2)) << "1 >= 2??";
  ARGUE_ASSERT(BUG, (1 < 2)) << fmt::format("{} >= {}?", 1, 2);
  ARGUE_ASSERT(BUG, ReturnsTrue(1, 2, 3)) << "Unexpected!";
  ARGUE_ASSERT(BUG, TReturnsTrue<int, int, int>(1, 2, 3)) << "Unexpected!";
}
