#pragma once
// Copyright 2021 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdint>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

namespace tangent {

// Stores the file name and line number of an assertion exception.
/* An assetion sentinel captures the current file and line number so that the
 * correct value is used, even if the message occurs at a later line. It also
 * serves as an indicator to the compiler which overload of operator&&() to
 * use during assertions. */
struct AssertionLocation {
  std::string filename;
  int line_no;
};

std::ostream& operator<<(std::ostream& out, const AssertionLocation& loc);

struct Exception : public std::exception {
 public:
  Exception(const std::string& msg, const AssertionLocation& loc)
      : message{msg}, location{loc} {}

  Exception(const std::string&& msg, const AssertionLocation& loc)
      : message{msg}, location{loc} {}

  Exception(const char* msg, const AssertionLocation& loc)
      : message{msg}, location{loc} {}

  virtual ~Exception() {}

  const char* what() const noexcept override;

  std::string message;
  AssertionLocation location;
};

std::ostream& operator<<(std::ostream& out, const Exception& ex);

// Exception message builder
/* Provides state and stream operators so that complex exception messages can
 * be conveniently composed. The purpose of an Assertion object is just to
 * hold the exception type and message used to construct an exception. The
 * exception itself is not constructructed and thrown until the
 * `operator&&` is called */
class Assertion {
 public:
  // message will be appended
  explicit Assertion(bool expr_value);

  // construct with message
  Assertion(bool expr_value, const std::string& message);

  template <typename T>
  Assertion& operator<<(const T& x);

  template <typename T>
  Assertion& operator<<(const T&& x);

  bool expr_value_;           //< The boolean expression being evaluated
  std::stringstream sstream;  //< stringstream to store the intermediate message
                              // as it's being constructed
};

// Construct and throw an exception
/* The && operator has lower precidence than the << operator so this allows
 * us to raise the exception after the message has been constructed but before
 * the assertion object has been destroyed.
 *
 * Usage:
 *
 * ```
 * AssertionLocation(...) && Assertion(...) << "Message";
 * ```
 */
void operator&&(const tangent::AssertionLocation& sentinel,
                const tangent::Assertion& assertion);

}  // namespace tangent

// Evaluate an assertion (boolean expression) and if false throw an exception
/* Use this macro is as if it was a function with the following signature:
 *
 * ```
 * ostream& TANGENT_ASSERT(Exception::severity class, bool expr);
 * ```
 * e.g.
 * ```
 * TANGENT_ASSERT(INPUT_ERROR, a == b)
 *  << "Message line 1\n"
 *  << "Message line 2\n";
 * ``` */
#define TANGENT_ASSERT(...)                             \
  ::tangent::AssertionLocation({__FILE__, __LINE__}) && \
      ::tangent::Assertion(__VA_ARGS__)

// Throw an exception using the exception message builder
/* Use this macro as if it was a function with following signatures:
 *
 * ```
 * ostream& TANGENT_THROW(Severity::severity class);
 * ```
 *
 * e.g.
 *
 * ```
 * TANGENT_THROW(INPUT_ERROR)
 *  << "Message line 1\n"
 *  << "Message line 2\n";
 * ``` */
#define TANGENT_THROW()                                 \
  ::tangent::AssertionLocation({__FILE__, __LINE__}) && \
      ::tangent::Assertion(false)

//
//
// =============================================================================
//                       Template Implementations
// =============================================================================
//
//

namespace tangent {

template <typename T>
Assertion& Assertion::operator<<(const T& x) {
  if (!expr_value_) {
    sstream << x;
  }
  return *this;
}

template <typename T>
Assertion& Assertion::operator<<(const T&& x) {
  if (!expr_value_) {
    sstream << x;
  }
  return *this;
}

}  // namespace tangent
