cc_test(
  argue-assert_test
  SRCS assert_test.cc
  DEPS argue gtest gtest_main
  LABELS CI-DISABLED)

cc_test(
  argue-arg_test
  SRCS arg_test.cc
  DEPS argue gtest gtest_main)

cc_test(
  argue-parse_test
  SRCS parse_test.cc
  DEPS argue gtest gtest_main)

cc_test(
  argue-util_test
  SRCS util_test.cc
  DEPS argue gtest gtest_main)

cc_test(
  argue-keyword_test
  SRCS keyword_test.cc
  DEPS argue gtest gtest_main)

add_custom_target(
  run.argue-execution_test
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/execution_tests.py --exe-path
          $<TARGET_FILE:argue-argparse-example>
  DEPENDS argue-argparse-example
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(
  NAME argue-execution_test
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/execution_tests.py --exe-path
          $<TARGET_FILE:argue-argparse-example>
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set_property(
  TEST argue-execution_test
  APPEND
  PROPERTY LABELS CI-DISABLED)
