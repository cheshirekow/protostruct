#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com
#include <sys/types.h>

// NOTE(josh):
// Linux Version (all one line, i.e. escape newlines)
// #define container_of(ptr, type, member)
//   ({
//     const typeof(((type*)0)->member)* __mptr = (ptr);
//     (type*)((char*)__mptr - offsetof(type, member));
//   })

// NOTE(josh): borrowed from
// http://shimpossible.blogspot.com/2013/08/containerof-and-offsetof-in-c.html
template <class P, class M>
constexpr size_t offset_of(const M P::*member) {
  return (size_t) & (reinterpret_cast<P*>(0)->*member);
}

// NOTE(josh): borrowed from
// http://shimpossible.blogspot.com/2013/08/containerof-and-offsetof-in-c.html
template <class P, class M>
P* container_of(M* ptr, const M P::*member) {
  return (P*)((char*)ptr - offset_of(member));  // NOLINT(readability/casting)
}

template <class P, class M>
constexpr P* container_of(const M* ptr, const M P::*member) {
  return (P*)((char*)ptr - offset_of(member));  // NOLINT(readability/casting)
}
