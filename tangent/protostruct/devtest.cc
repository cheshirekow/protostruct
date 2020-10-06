#include <gtest/gtest.h>

#include "tangent/protostruct/test/test_messages.h"
#include "tangent/protostruct/test/test_messages.pb.h"
#include "tangent/protostruct/test/test_messages.pbwire.h"

std::string to_hex(const std::string& str) {
  std::stringstream strm{};
  for (size_t idx = 0; idx < str.length(); idx++) {
    if (idx > 0) {
      strm << " ";
    }
    uint8_t byte{};
    memcpy(&byte, &str[idx], 1);
    strm << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase
         << (int)byte;
  }
  return strm.str();
}

TEST(Protostruct, TestSerialization) {
  tangent::test::MyMessageA proto{};
  proto.set_fielda(-12);
  proto.set_fieldb(15.72);
  proto.set_fieldc(13579);
  proto.set_fieldd(tangent::test::MyEnumA_VALUE2);
  std::string serialized_proto = proto.SerializeAsString();

  MyMessageA cmsg{};
  cmsg.fieldA = -12;
  cmsg.fieldB = 15.72;
  cmsg.fieldC = 13579;
  cmsg.fieldD = MyEnumA_VALUE2;

  pbwire_Error error{};
  pbwire_EmitContext ectx{};
  ASSERT_EQ(0, pbwire_emitcontext_init(&ectx, 10, 100, &error));

  int resultcode = pbemit_MyMessageA(ectx, &cmsg);
  ASSERT_LT(0, resultcode) << error.msg;
  //  ASSERT_EQ(serialized_proto.size(), resultcode);

  std::string serialized_struct{ectx.buffer.begin,
                                static_cast<size_t>(resultcode)};
  ASSERT_EQ(0, pbwire_emitcontext_cleanup(&ectx));
  EXPECT_EQ(serialized_proto, serialized_struct)
      << "   serialized_proto: " << to_hex(serialized_proto)
      << "\n  serialized_struct: " << to_hex(serialized_struct);

  pbwire_ParseContext pctx;
  pctx.buffer.begin = &serialized_proto[0];
  pctx.buffer.ptr = pctx.buffer.begin;
  pctx.buffer.end = &serialized_proto.back() + 1;
  pctx.error = &error;

  memset(&cmsg, 0, sizeof(cmsg));
  resultcode = pbparse_MyMessageA(pctx, &cmsg);
  ASSERT_LE(0, resultcode) << error.msg << "\nNote, buffer size is: "
                           << serialized_proto.size();
  EXPECT_EQ(serialized_proto.size(), resultcode);
  EXPECT_EQ(-12, cmsg.fieldA);
  EXPECT_EQ(15.72, cmsg.fieldB);
  EXPECT_EQ(13579, cmsg.fieldC);
}
