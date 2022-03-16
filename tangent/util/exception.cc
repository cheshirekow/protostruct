// Copyright 2021 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/util/exception.h"

namespace tangent {

std::ostream& operator<<(std::ostream& out, const AssertionLocation& loc) {
  out << loc.filename << "[" << loc.line_no << "]";
  return out;
}

std::ostream& operator<<(std::ostream& out, const Exception& ex) {
  out << "Exception at " << ex.location << ": " << ex.message;
  return out;
}

// message will be appended
Assertion::Assertion(bool expr_value) : expr_value_(expr_value) {}

// construct with message
Assertion::Assertion(bool expr_value, const std::string& message)
    : expr_value_(expr_value) {
  if (!expr_value_) {
    sstream << message;
  }
}

void operator&&(const tangent::AssertionLocation& sentinel,
                const tangent::Assertion& assertion) {
  if (!assertion.expr_value_) {
    throw tangent::Exception{assertion.sstream.str(), sentinel};
  }
}

const char* Exception::what() const noexcept {
  return message.c_str();
}

}  // namespace tangent
