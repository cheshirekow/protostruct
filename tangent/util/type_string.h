#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com
#include <cstring>
#include <string>

template <typename _TypeName>
inline std::string hacky_type_string() {
  const char* fullstr = __PRETTY_FUNCTION__;
  const char* begin = strstr(fullstr, "_TypeName =");
  const char* end = strstr(begin, ";");
  if (begin) {
    begin += 12;
    // TODO(josh): should we just return begin? It has static storage duration
    // so it's super fast and convenient to just access it and we don't have to
    // alloc/dealloc anything. But then we have an extra ']' at the end of the
    // string which is somewhat annoying.
    if (end) {
      return std::string{begin, static_cast<size_t>(end - begin)};
    }
    return std::string{begin, strlen(begin) - 1};
  }
  return fullstr;
}

#if GNU
#include <cxxabi.h>

template <typename T>
inline std::string type_string() {
  std::string out;

  char* name = nullptr;
  int status = 0;
  abie::__cxa_demangle(typeid(U).name(), 0, 0, &status);
  if (!status) {
    std::string out{name};
    free(name);
    return std::move(out);
  }

  return typeid(U).name();
}

#else

template <typename T>
inline std::string type_string() {
  return hacky_type_string<T>();
}

#endif

#define DECL_TYPESTRING(x)              \
  template <>                           \
  inline std::string type_string<x>() { \
    return #x;                          \
  }

DECL_TYPESTRING(int8_t);
DECL_TYPESTRING(int16_t);
DECL_TYPESTRING(int32_t);
DECL_TYPESTRING(int64_t);
DECL_TYPESTRING(uint8_t);
DECL_TYPESTRING(uint16_t);
DECL_TYPESTRING(uint32_t);
DECL_TYPESTRING(uint64_t);
DECL_TYPESTRING(std::string);
