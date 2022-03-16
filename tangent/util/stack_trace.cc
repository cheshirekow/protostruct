// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include "tangent/util/stack_trace.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <algorithm>

namespace util {
// =============================================================================
//                    Exception Handling and Stack Traces
// =============================================================================

// Parse the textual content of a stacktrace into their individual components.
// A line within a stacktrace has format:
// (<name>+<offset>) [<address>]
static void ParseTraceLine(char* symbol, TraceLine* traceline) {
  char* begin_name = 0;
  char* begin_offset = 0;
  char* end_offset = 0;
  char* begin_addr = 0;
  char* end_addr = 0;

  // Find parentheses and +address offset surrounding the mangled name:
  // ./module(function+0x15c) [0x8048a6d]
  for (char* ptr = symbol; *ptr; ++ptr) {
    if (*ptr == '(') {
      begin_name = ptr;
    } else if (*ptr == '+') {
      begin_offset = ptr;
    } else if (*ptr == ')' && begin_offset) {
      end_offset = ptr;
    } else if (*ptr == '[' && end_offset) {
      begin_addr = ptr;
    } else if (*ptr == ']' && begin_addr) {
      end_addr = ptr;
    }
  }

  if (begin_name) {
    *begin_name = '\0';
  }
  traceline->file = symbol;

  if (begin_offset) {
    *begin_offset = '\0';
  }
  if (end_offset) {
    *end_offset = '\0';
  }

  if (begin_offset && begin_name < begin_offset) {
    traceline->name = (begin_name + 1);
    traceline->offset = (begin_offset + 1);
  }

  if (end_addr) {
    *end_addr = '\0';
    traceline->saddr = (begin_addr + 1);
  }
}

// TODO(josh): use libunwind
StackTrace GetStacktrace(size_t skip_frames, size_t max_frames) {
  StackTrace result;
  std::vector<void*> addrlist(max_frames + 1);

  // http://man7.org/linux/man-pages/man3/backtrace.3.html
  int addrlen = backtrace(&addrlist[0], addrlist.size());
  if (addrlen == 0) {
    return {{0, "<empty, possibly corrupt>"}};
  }

  // symbol -> "filename(function+offset)" (this array needs to free())
  char** symbols = backtrace_symbols(&addrlist[0], addrlen);

  // We have to malloc this, can't use std::string(), because demangle function
  // may realloc() it.
  size_t funcnamesize = 256;
  char* funcnamebuf = static_cast<char*>(malloc(funcnamesize));

  // Iterate over the returned symbol lines, skipping frames as requested,
  // and one extra for this function.
  result.reserve(addrlen - skip_frames);
  for (size_t idx = skip_frames + 1; idx < static_cast<size_t>(addrlen);
       idx++) {
    TraceLine traceline{.addr = addrlist[idx]};
    ParseTraceLine(symbols[idx], &traceline);
    int status = 0;
    if (traceline.name.size()) {
      char* ret = abi::__cxa_demangle(traceline.name.c_str(), funcnamebuf,
                                      &funcnamesize, &status);
      if (status == 0) {
        funcnamebuf = ret;
        traceline.name = funcnamebuf;
      }
    }

    result.push_back(traceline);
  }

  free(funcnamebuf);
  free(symbols);
  return result;
}

}  // namespace util
