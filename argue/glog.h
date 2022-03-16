#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/parser.h"

namespace argue {

// Add glog options (normally exposed through gflags) to the parser
void add_glog_options(Parser* parser);

}  // namespace argue
