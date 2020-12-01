#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <cstdint.h>
#include <stdbool.h>

struct Foo {
  bool fooA;
  bool fooB;
};

struct Bar {
  Foo barA;
  int barB;
  bool barC;
};
