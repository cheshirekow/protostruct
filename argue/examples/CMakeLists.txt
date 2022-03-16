cc_binary(
  argue-argparse-example
  SRCS argparse_example.cc
  DEPS argue)

cc_binary(
  argue-simple-example
  SRCS simple_example.cc
  DEPS argue)

cc_binary(
  argue-subparser-example
  SRCS subparser_example.cc
  DEPS argue)
