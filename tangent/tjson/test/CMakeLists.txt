cc_test(
  tjson-lexer_test
  SRCS lexer_test.cc
  DEPS gtest gtest_main tjson
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  tjson-parser_test
  SRCS parser_test.cc
  DEPS gtest gtest_main tjson
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  tjson-string_test
  SRCS string_test.cc
  DEPS gtest gtest_main tjson
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

cc_test(
  tjson-ostream_test
  SRCS ostream_test.cc
  DEPS gtest gtest_main tjson tangent::util
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
