// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <iostream>
#include <list>
#include <memory>

#include "argue/argue.h"

class Accumulator {
 public:
  std::string GetName() {
    return name_;
  }
  virtual int operator()(const std::list<int>& args) = 0;

 protected:
  std::string name_;
};

struct Max : public Accumulator {
  Max() {
    name_ = "max";
  }

  virtual ~Max() {}

  int operator()(const std::list<int>& args) override {
    if (args.size() == 0) {
      return 0;
    }
    int result = args.front();
    for (int x : args) {
      if (x > result) {
        result = x;
      }
    }
    return result;
  }
};

struct Sum : public Accumulator {
  Sum() {
    name_ = "sum";
  }

  virtual ~Sum() {}

  int operator()(const std::list<int>& args) override {
    int result = 0;
    for (int x : args) {
      result += x;
    }
    return result;
  }
};

int main(int argc, char** argv) {
  std::list<int> int_args;
  std::shared_ptr<Accumulator> accumulate;
  std::shared_ptr<Accumulator> sum_fn = std::make_shared<Sum>();
  std::shared_ptr<Accumulator> max_fn = std::make_shared<Max>();

  argue::Parser parser({
      .add_help = true,
      .add_version = true,
      .name = "argue-demo",
      .version = argue::VersionString ARGUE_VERSION,
      .author = "Josh Bialkowski <josh.bialkowski@gmail.com>",
      .copyright = "(C) 2018",
  });

  using namespace argue::keywords;  // NOLINT

  // clang-format off
  parser.add_argument(
      "integer", nargs="+", choices={1, 2, 3, 4}, dest=&int_args,  // NOLINT
      help="an integer for the accumulator", metavar="N");         // NOLINT

  parser.add_argument(
    "-s", "--sum", action="store_const", dest=&accumulate,  // NOLINT
    const_=sum_fn, default_=max_fn,                         // NOLINT
    help="sum the integers (default: find the max)");       // NOLINT
  // clang-format on

  int parse_result = parser.parse_args(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  std::cout << accumulate->GetName() << "(" << stringutil::join(int_args)
            << ") = " << (*accumulate)(int_args) << "\n";
  return 0;
}
