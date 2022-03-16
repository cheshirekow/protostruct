#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <string>
#include <vector>

namespace util {
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
StackTrace get_stack_trace(size_t skip_frames = 1, size_t max_frames = 50);

// Print the stack trace line by line to the output stream.
std::ostream& operator<<(std::ostream& out, const StackTrace& trace);

}  // namespace util
