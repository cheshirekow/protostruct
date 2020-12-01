// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

#include "tangent/protostruct/test/test_messages.cereal.h"
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
         << static_cast<int>(byte);
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
  uint32_t length_cache[10];
  char data[100];

  pbwire_EmitContext ectx{};
  ectx.error = &error;
  pbwire_writebuffer_init(&ectx.buffer, data, &data[100]);
  pbwire_lengthcache_init(&ectx.length_cache, length_cache, &length_cache[10]);

  int bytes_written = pbemit_MyMessageA(&ectx, &cmsg);
  ASSERT_LT(0, bytes_written) << error.msg;
  //  ASSERT_EQ(serialized_proto.size(), bytes_written);

  std::string serialized_struct{data, static_cast<size_t>(bytes_written)};
  EXPECT_EQ(serialized_proto, serialized_struct)
      << "   serialized_proto: " << to_hex(serialized_proto)
      << "\n  serialized_struct: " << to_hex(serialized_struct);

  pbwire_ParseContext pctx{};
  pctx.error = &error;
  pbwire_readbuffer_init(&pctx.buffer, &serialized_proto[0],
                         &serialized_proto.back() + 1);

  memset(&cmsg, 0, sizeof(cmsg));
  int bytes_read = pbparse_MyMessageA(&pctx, &cmsg);
  ASSERT_LE(0, bytes_read) << error.msg << "\nNote, buffer size is: "
                           << serialized_proto.size();
  EXPECT_EQ(serialized_proto.size(), bytes_read);
  EXPECT_EQ(-12, cmsg.fieldA);
  EXPECT_EQ(15.72, cmsg.fieldB);
  EXPECT_EQ(13579, cmsg.fieldC);
}

TEST(Protostruct, TestCerealJSON) {
  MyMessageA cmsg{};
  cmsg.fieldA = -12;
  cmsg.fieldB = 15.72;
  cmsg.fieldC = 13579;
  cmsg.fieldD = MyEnumA_VALUE2;

  std::stringstream strm{};
  /* archive scope */ {
    cereal::JSONOutputArchive oarchive{strm};
    ::serialize(oarchive, cmsg);
  }

  std::string serialized_msg = strm.str();
  EXPECT_EQ(serialized_msg, R"({
    "fieldA": -12,
    "fieldB": 15.72,
    "fieldC": 13579,
    "fieldD": 1
})");

  strm.str(serialized_msg);
  memset(&cmsg, 0, sizeof(cmsg));

  /*archive scope */ {
    cereal::JSONInputArchive iarchive{strm};
    ::serialize(iarchive, cmsg);
  }

  EXPECT_EQ(-12, cmsg.fieldA);
  EXPECT_EQ(15.72, cmsg.fieldB);
  EXPECT_EQ(13579, cmsg.fieldC);
  EXPECT_EQ(MyEnumA_VALUE2, cmsg.fieldD);
}
