#pragma once
// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <ostream>
#include <streambuf>

class NullBuf : public std::streambuf {
 public:
  int overflow(int c) {
    return c;
  }
};

class NullStream : public std::ostream {
 public:
  NullStream() : std::ostream(&m_sb) {}

 private:
  NullBuf m_sb;
};
