// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <fstream>

#include <gtest/gtest.h>

#include "argue/argue.h"

#if __clang__
#define USE_DESIGNATED_INITIALIZERS 1
#elif __GNUC__
#if __GNUC__ > 5 && __cplusplus > 201400L
#define USE_DESIGNATED_INITIALIZERS 1
#else
#define USE_DESIGNATED_INITIALIZERS 0
#endif
#else
#define USE_DESIGNATED_INITIALIZERS 0
#endif

void ResetParser(argue::Parser* parser, const argue::Parser::Metadata& meta) {
  using argue::Parser;
  (parser)->~Parser();
  new (parser) Parser(meta);
}

void ResetParser(argue::Parser* parser) {
  using argue::Parser;
  (parser)->~Parser();
  new (parser) Parser();
}

TEST(StoreTest, StoreScalar) {
  std::ofstream nullstream{"/dev/null"};
  int foo = 0;
  argue::Parser parser;

  // Expect failure if we have too few args, manditory positional argument
  // remains
  ResetParser(&parser);
  parser.add_argument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({}, &nullstream));
  EXPECT_EQ(0, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.add_argument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({"1", "2"}, &nullstream));
  EXPECT_EQ(1, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.add_argument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"1"}, &nullstream));
  EXPECT_EQ(1, foo);

  // Flags default optional so an empty args list should be OK
  ResetParser(&parser);
  parser.add_argument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({}, &nullstream));
  EXPECT_EQ(0, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.add_argument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({"1", "2"}, &nullstream));
  EXPECT_EQ(0, foo);

  ResetParser(&parser);
  parser.add_argument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"-f", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  ResetParser(&parser);
  parser.add_argument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"--foo", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  // Ensure that flag deduction works correctly
  ResetParser(&parser);
  parser.add_argument("-f", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"-f", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  ResetParser(&parser);
  parser.add_argument("--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"--foo", "1"}, &nullstream));
  EXPECT_EQ(1, foo);

  // If argument is optional then parse should not fail on empty string
  ResetParser(&parser);
  parser.add_argument("foo", &foo,
                      {.action = "store", .nargs = argue::ZERO_OR_ONE});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({}, &nullstream));
  EXPECT_EQ(0, foo);
}

TEST(StoreTest, StoreTypes) {
  std::ofstream nullstream{"/dev/null"};
  argue::Parser parser;

  ResetParser(&parser);
  int32_t i32_foo = 0;
  parser.add_argument("foo", &i32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"123"}, &nullstream));
  EXPECT_EQ(123, i32_foo);

  ResetParser(&parser);
  uint32_t u32_foo = 0;
  parser.add_argument("foo", &u32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"123"}, &nullstream));
  EXPECT_EQ(123, u32_foo);

  ResetParser(&parser);
  float f32_foo = 0;
  parser.add_argument("foo", &f32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"123"}, &nullstream));
  EXPECT_EQ(123, f32_foo);

  ResetParser(&parser);
  double f64_foo = 0;
  parser.add_argument("foo", &f64_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"123"}, &nullstream));
  EXPECT_EQ(123, f64_foo);

  ResetParser(&parser);
  std::string str_foo = "hello";
  parser.add_argument("foo", &str_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"123"}, &nullstream));
  EXPECT_EQ("123", str_foo);
}

TEST(StoreTest, StoreOneOrMore) {
  std::ofstream nullstream{"/dev/null"};
  std::stringstream logstream{};
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list should fail if the requirement is one or more
  ResetParser(&parser);
#if USE_DESIGNATED_INITIALIZERS
  parser.add_argument<int>("foo",
                           {.action = "store",  //
                            .nargs = argue::ONE_OR_MORE,
                            .dest = &container});
#else
  {
    auto act =
        parser.add_argument("foo", argue::KWargs<int>{.action = "store"});
    act.nargs = argue::ONE_OR_MORE;
    act.dest = &container;
  }
#endif
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({}, &nullstream));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
#if USE_DESIGNATED_INITIALIZERS
  parser.add_argument<int>("foo",
                           {.action = "store",  //
                            .nargs = argue::ONE_OR_MORE,
                            .dest = &container});
#else
  {
    auto act = parser.add_argument(
        "foo",
        argue::KWargs<int>{.action = "store", .nargs = argue::ONE_OR_MORE});
    act.dest = &container;
  }
#endif
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"1"}, &logstream))
      << logstream.str();
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
#if USE_DESIGNATED_INITIALIZERS
  parser.add_argument<int>("foo",
                           {.action = "store",  //
                            .nargs = argue::ONE_OR_MORE,
                            .dest = &container});
#else
  {
    auto act = parser.add_argument(
        "foo",
        argue::KWargs<int>{.action = "store", .nargs = argue::ONE_OR_MORE});
    act.dest = &container;
  }
#endif
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"1", "2", "3"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.add_argument("foo", &container,
                      {.action = "store",  //
                       .nargs = argue::ONE_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"1", "2", "3"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
}

TEST(StoreTest, StoreZeroOrMore) {
  std::ofstream nullstream{"/dev/null"};
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list is allowed if specification is for zero or more
  ResetParser(&parser);
  parser.add_argument("foo", &container,
                      {.action = "store", .nargs = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({}, &nullstream));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.add_argument("foo", &container,
                      {.action = "store", .nargs = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"1"}, &nullstream));
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.add_argument("foo", &container,
                      {.action = "store", .nargs = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"1", "2", "3"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
}

TEST(StoreTest, StoreFixedSize) {
  std::ofstream nullstream{"/dev/null"};
  std::stringstream strm;
  int dummy;
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list is allowed if specification is for zero or more
  // ResetParser(&parser);
  // EXPECT_THROW(
  //   parser.add_argument("foo", &container, {.action = "store", .nargs = 0}),
  //   argue::Exception);
  // container = {};
  // EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({}));
  // EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.add_argument("foo", &container, {.action = "store", .nargs = 1});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"1"}, &strm))
      << strm.str();
  strm.str("");
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.add_argument("foo", &container, {.action = "store", .nargs = 1});
  parser.add_argument("bar", &dummy, {});
  container = {};
  dummy = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.parse_args({"1", "2"}, &nullstream));
  expected = {1};
  EXPECT_EQ(expected, container);
  EXPECT_EQ(2, dummy);

  ResetParser(&parser);
  parser.add_argument("foo", &container, {.action = "store", .nargs = 3});
  parser.add_argument("bar", &dummy, {});
  container = {};
  dummy = 0;
  EXPECT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"1", "2", "3", "4"}, &nullstream));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
  EXPECT_EQ(4, dummy);

  ResetParser(&parser);
  parser.add_argument("foo", &container, {.action = "store", .nargs = 4});
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({"1", "2"}, &nullstream));
  expected = {1, 2};
  EXPECT_EQ(expected, container);
}

TEST(HelpTest, HelpIsDefault) {
  std::ofstream nullstream{"/dev/null"};
  std::stringstream strm;
  argue::Parser parser;
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.parse_args({"--help"}, &strm))
      << strm.str();
  strm.str("");
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.parse_args({"-h"}, &strm))
      << strm.str();
  strm.str("");

  ResetParser(&parser, {.add_help = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({"--help"}, &nullstream));
  ResetParser(&parser, {.add_help = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({"-h"}, &nullstream));
}

TEST(VersionTest, VersionIsDefault) {
  std::ofstream nullstream{"/dev/null"};
  argue::Parser parser;
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED,
            parser.parse_args({"--version"}, &nullstream));

  ResetParser(&parser, {.add_help = true, .add_version = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION,
            parser.parse_args({"--version"}, &nullstream));
}

struct TestOpts {
  std::string command;
  std::string foo;

  struct {
    std::string a;
    std::string b;
  } bar;

  struct {
    std::string c;
    std::string d;
  } baz;
};

TEST(SubparserTest, SubparsersWork) {
  TestOpts opts;

  std::stringstream logout;
  argue::Parser parser;
  ResetParser(&parser);
  parser.add_argument("-f", "--foo", &opts.foo, {});
  auto subparsers = parser.add_subparsers("command", &opts.command);
  ASSERT_NE(nullptr, subparsers.get());
  auto bar_parser = subparsers->add_parser("bar");
  ASSERT_NE(nullptr, bar_parser.get());
  bar_parser->add_argument("-a", &opts.bar.a);
  bar_parser->add_argument("-b", &opts.bar.b);
  auto baz_parser = subparsers->add_parser("baz");
  ASSERT_NE(nullptr, baz_parser.get());
  baz_parser->add_argument("-c", &opts.baz.c);
  baz_parser->add_argument("-d", &opts.baz.d);

  ASSERT_EQ(argue::PARSE_FINISHED, parser.parse_args({"bar"}, &logout))
      << logout.str();
  EXPECT_EQ("bar", opts.command);

  logout.str("");
  ASSERT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"bar", "-a", "hello", "-b", "world"}, &logout))
      << logout.str();
  EXPECT_EQ("bar", opts.command);
  EXPECT_EQ("hello", opts.bar.a);
  EXPECT_EQ("world", opts.bar.b);

  logout.str("");
  ASSERT_EQ(argue::PARSE_EXCEPTION,
            parser.parse_args({"bar", "-c", "hello"}, &logout))
      << logout.str();

  logout.str("");
  ASSERT_EQ(argue::PARSE_FINISHED, parser.parse_args({"baz"}, &logout))
      << logout.str();

  logout.str("");
  ASSERT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"baz", "-c", "hello", "-d", "world"}, &logout))
      << logout.str();
  EXPECT_EQ("baz", opts.command);
  EXPECT_EQ("hello", opts.baz.c);
  EXPECT_EQ("world", opts.baz.d);

  logout.str("");
  ASSERT_EQ(argue::PARSE_EXCEPTION,
            parser.parse_args({"baz", "-a", "-b"}, &logout))
      << logout.str();

  logout.str("");
  ASSERT_EQ(
      argue::PARSE_FINISHED,
      parser.parse_args({"--foo", "hello", "bar", "-a", "hello", "-b", "world"},
                        &logout))
      << logout.str();
}

TEST(FlagTest, MatchUniquePrefix) {
  argue::Parser parser;
  ResetParser(&parser);
  bool do_optional_thing{false};
  bool do_other_thing{false};

  parser.add_argument("--do-optional-thing", &do_optional_thing,
                      {.action = "store_true"});
  parser.add_argument("--do-other-thing", &do_other_thing,
                      {.action = "store_true"});

  std::stringstream logstrm;
  ASSERT_EQ(argue::PARSE_FINISHED,
            parser.parse_args({"--do-optional"}, &logstrm))
      << logstrm.str();
  ASSERT_EQ(argue::PARSE_EXCEPTION, parser.parse_args({"--do"}, &logstrm))
      << logstrm.str();
}
