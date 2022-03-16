// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <array>

#include <gtest/gtest.h>

#include "tangent/tjson/tjson.h"

std::array<tjson_Token, 255> g_token_store_;
std::array<tjson_Event, 255> g_event_store_;

TEST(ParserTest, TestKnownParsings) {
  tjson_Error error{};
  tjson_StringPiece test_piece = tjson_StringPiece_fromstr(
      "{\"foo\":{\"bar\":1,\"baz\":[\"a\",1,12.3,true,false,null]}}");

  int nevents = tjson_parse(test_piece, &g_event_store_[0],
                            g_event_store_.size(), &error);
  ASSERT_EQ(16, nevents) << error.msg;
  ASSERT_EQ(TJSON_OBJECT_BEGIN, g_event_store_[0].typeno);
  ASSERT_EQ(TJSON_OBJECT_KEY, g_event_store_[1].typeno);
  ASSERT_EQ(TJSON_OBJECT_BEGIN, g_event_store_[2].typeno);
  ASSERT_EQ(TJSON_OBJECT_KEY, g_event_store_[3].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[4].typeno);
  ASSERT_EQ(TJSON_OBJECT_KEY, g_event_store_[5].typeno);
  ASSERT_EQ(TJSON_LIST_BEGIN, g_event_store_[6].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[7].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[8].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[9].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[10].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[11].typeno);
  ASSERT_EQ(TJSON_VALUE_LITERAL, g_event_store_[12].typeno);
  ASSERT_EQ(TJSON_LIST_END, g_event_store_[13].typeno);
  ASSERT_EQ(TJSON_OBJECT_END, g_event_store_[14].typeno);
  ASSERT_EQ(TJSON_OBJECT_END, g_event_store_[15].typeno);
}
