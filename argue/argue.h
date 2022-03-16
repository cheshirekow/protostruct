#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/action.h"
#include "argue/exception.h"
#include "argue/keywords.h"
#include "argue/kwargs.h"
#include "argue/parse.h"
#include "argue/parser.h"
#include "argue/storage_model.h"
#include "argue/util.h"

#include "argue/action.tcc"
#include "argue/keywords.tcc"
#include "argue/kwargs.tcc"
#include "argue/parse.tcc"
#include "argue/parser.tcc"
#include "argue/storage_model.tcc"

#define ARGUE_VERSION \
  { 0, 1, 4, "dev", 0 }

// An command line argument parsing library
namespace argue {}  // namespace argue
