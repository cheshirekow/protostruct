#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <sstream>
#include <string>
#include <vector>

namespace argue {

// =============================================================================
//                    Exception Handling and Stack Traces
// =============================================================================

// Element type for a stack trace
// see: http://man7.org/linux/man-pages/man3/backtrace.3.html
struct TraceLine {
  void* addr;          //< symbol address
  std::string file;    //< file from which the symbol originated
  std::string name;    //< symbol name
  std::string offset;  //< string representation of the stack pointer offset
                       // from the symbol
  std::string saddr;   //< string representation of the symbol address
};

// A stack trace is just a vector of stack line information
typedef std::vector<TraceLine> StackTrace;

// Return the current stack trace. ``skip_frames`` defaults to 2 because
// the argue::Assertion adds two calls to the stack.
StackTrace get_stacktrace(size_t skip_frames = 1, size_t max_frames = 50);

// Print the stack trace line by line to the output stream.
std::ostream& operator<<(std::ostream& out, const StackTrace& trace);

// Exceptions thrown by this library are all of this type.
class Exception : public std::exception {
 public:
  // Signify severity of the exception, and whether it's an exception due
  // to argue itself, the library user who is writing a parser, or the
  // program user who is calling the program with arguments.
  enum TypeNo {
    BUG = 0,       // A bug in the library code
    CONFIG_ERROR,  // Library user error
    INPUT_ERROR,   // Program user error
  };

  Exception(TypeNo typeno, const std::string& message)
      : typeno(typeno), message(message) {}
  const char* what() const noexcept override;
  static const char* to_string(TypeNo typeno);

  TypeNo typeno;
  std::string file;
  int lineno;
  std::string message;
  StackTrace stack_trace;
};

// Stores the file name and line number of an assertion exception.
/* An assetion sentinel captures the current file and line number so that the
 * correct value is used, even if the message occurs at a later line. It also
 * serves as an indicator to the compiler which overload of operator&&() to
 * use during assertions. */
struct AssertionSentinel {
  std::string file;
  int lineno;
};

// Exception message builder
/* Provides state and stream operators so that complex exception messages can
 * be conveniently composed. The purpose of an Assertion object is just to
 * hold the exception type and message used to construct an exception. The
 * exception itself is not constructructed and thrown until the
 * `operator&&` is called */
class Assertion {
 public:
  // message will be appended
  explicit Assertion(Exception::TypeNo typeno, bool expr);

  // construct with message
  Assertion(Exception::TypeNo typeno, bool expr, const std::string& message);

  template <typename T>
  Assertion& operator<<(const T& x);

  template <typename T>
  Assertion& operator<<(const T&& x);

  Exception::TypeNo typeno;   //< The exception type number
  bool expr_;                 //< The boolean expression being evaluated
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
 * AssertionSentinel(...) && Assertion(...) << "Message";
 * ```
 */
void operator&&(const argue::AssertionSentinel& sentinel,
                const argue::Assertion& assertion);

}  // namespace argue

// Evaluate an assertion (boolean expression) and if false throw an exception
/* Use this macro is as if it was a function with the following signature:
 *
 * ```
 * ostream& ARGUE_ASSERT(Exception::TypeNo class, bool expr);
 * ```
 * e.g.
 * ```
 * ARGUE_ASSERT(INPUT_ERROR, a == b)
 *  << "Message line 1\n"
 *  << "Message line 2\n";
 * ``` */
#define ARGUE_ASSERT(typeno, ...)                     \
  ::argue::AssertionSentinel({__FILE__, __LINE__}) && \
      ::argue::Assertion(::argue::Exception::typeno, __VA_ARGS__)

// Throw an exception using the exception message builder
/* Use this macro as if it was a function with following signatures:
 *
 * ```
 * ostream& ARGUE_THROW(Exception::TypeNo class);
 * ```
 *
 * e.g.
 *
 * ```
 * ARGUE_THROW(INPUT_ERROR)
 *  << "Message line 1\n"
 *  << "Message line 2\n";
 * ``` */
#define ARGUE_THROW(typeno)                           \
  ::argue::AssertionSentinel({__FILE__, __LINE__}) && \
      ::argue::Assertion(::argue::Exception::typeno, false)

//
//
// =============================================================================
//                       Template Implementations
// =============================================================================
//
//

namespace argue {

template <typename T>
Assertion& Assertion::operator<<(const T& x) {
  if (!expr_) {
    sstream << x;
  }
  return *this;
}

template <typename T>
Assertion& Assertion::operator<<(const T&& x) {
  if (!expr_) {
    sstream << x;
  }
  return *this;
}

}  // namespace argue
