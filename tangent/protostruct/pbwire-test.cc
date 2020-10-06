#include <gtest/gtest.h>

#include "tangent/protostruct/pbwire.h"

TEST(pbwireTest, TestZigZag) {
  EXPECT_EQ(0, pbwire_zigzag32(0));
  EXPECT_EQ(1, pbwire_zigzag32(-1));
  EXPECT_EQ(2, pbwire_zigzag32(1));
  EXPECT_EQ(3, pbwire_zigzag32(-2));
  EXPECT_EQ(4294967294, pbwire_zigzag32(2147483647));
  EXPECT_EQ(4294967295, pbwire_zigzag32(-2147483648));

  EXPECT_EQ(0, pbwire_zigzag64(0));
  EXPECT_EQ(1, pbwire_zigzag64(-1));
  EXPECT_EQ(2, pbwire_zigzag64(1));
  EXPECT_EQ(3, pbwire_zigzag64(-2));
  EXPECT_EQ(4294967294, pbwire_zigzag64(2147483647));
  EXPECT_EQ(4294967295, pbwire_zigzag64(-2147483648));

  EXPECT_EQ(0, pbwire_unzigzag32(0));
  EXPECT_EQ(-1, pbwire_unzigzag32(1));
  EXPECT_EQ(1, pbwire_unzigzag32(2));
  EXPECT_EQ(-2, pbwire_unzigzag32(3));
  EXPECT_EQ(2147483647, pbwire_unzigzag32(4294967294));
  EXPECT_EQ(-2147483648, pbwire_unzigzag32(4294967295));

  EXPECT_EQ(0, pbwire_unzigzag64(0));
  EXPECT_EQ(-1, pbwire_unzigzag64(1));
  EXPECT_EQ(1, pbwire_unzigzag64(2));
  EXPECT_EQ(-2, pbwire_unzigzag64(3));
  EXPECT_EQ(2147483647, pbwire_unzigzag64(4294967294));
  EXPECT_EQ(-2147483648, pbwire_unzigzag64(4294967295));
}

TEST(pbwireTest, TestParseVarint) {
  char data[32] = {0};

  pbwire_Error error{};
  pbwire_ParseContext pctx{};
  pctx.buffer.begin = data;
  pctx.buffer.ptr = data;
  pctx.buffer.end = data + sizeof(data);
  pctx.error = &error;

  uint32_t value{0};
  int result{0};

  memset(data, 0, sizeof(data));
  strncpy(data, "\x01", sizeof(data));
  result = pbwire_parse_varint32(pctx, &value);
  ASSERT_LE(0, result) << error.msg;
  ASSERT_EQ(1, result);
  ASSERT_EQ(1, value);

  memset(data, 0, sizeof(data));
  strncpy(data, "\xac\x02", sizeof(data));
  result = pbwire_parse_varint32(pctx, &value);
  ASSERT_LE(0, result) << error.msg;
  ASSERT_EQ(2, result);
  ASSERT_EQ(300, value);
}

TEST(pbwireTest, TestEmitVarint) {
  char data[32] = {0};

  pbwire_Error error{};
  pbwire_EmitContext ectx{};
  ectx.buffer.begin = data;
  ectx.buffer.ptr = data;
  ectx.buffer.end = data + sizeof(data);
  ectx.error = &error;

  int result{0};

  memset(data, 0, sizeof(data));
  result = pbwire_emit_varint32(ectx, 1);
  ASSERT_LE(0, result) << error.msg;
  ASSERT_EQ(1, result);
  ASSERT_EQ('\x01', data[0]);

  memset(data, 0, sizeof(data));
  result = pbwire_emit_varint32(ectx, 300);
  ASSERT_LE(0, result) << error.msg;
  ASSERT_EQ(2, result);
  ASSERT_EQ('\xac', data[0]);
  ASSERT_EQ('\x02', data[1]);
}
