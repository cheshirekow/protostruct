// Copyright (C) 2021 Josh Bialkowski (josh.bialkowski@gmail.com)
#include <gtest/gtest.h>

#include "tangent/util/hash.h"

TEST(Hash, RuntimeHashMatchesCompileTimeHash) {
  EXPECT_EQ(tangent::runtime_hash("hello world"), tangent::hash("hello world"));
}

TEST(Hash, CaseInsensitiveHashIsCaseInsensitive) {
  EXPECT_EQ(tangent::runtime_ci_hash("hello world"),
            tangent::ci_hash("HeLLo WorLd"));
}

TEST(Hash, StrippedUnderscoreHashStripsUnderscores) {
  EXPECT_EQ(tangent::runtime_suci_hash("hello_world"),
            tangent::suci_hash("HeLLo_WorLd"));
}
