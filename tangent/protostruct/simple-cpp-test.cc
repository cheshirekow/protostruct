// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "tangent/protostruct/test/test_messages-simple.h"

TEST(SimpleCpp, ToJSONMatchesExpectedFormat) {
  tangent::test::MyMessageC msg{};
  msg.fieldA.emplace_back();
  msg.fieldA[0].fieldA = 1;
  msg.fieldA[0].fieldB = 2.0;
  msg.fieldA[0].fieldC = 3;
  msg.fieldA[0].fieldD = tangent::test::MyEnumA_VALUE3;
  msg.fieldB.push_back(4);
  const std::string serial_value = msg.to_json();
  EXPECT_EQ(msg.to_json(), R"({
    "fieldA": [
        {
          "fieldA": 1,
          "fieldB": 2,
          "fieldC": 3,
          "fieldD": "MyEnumA_VALUE3"
      }
    ],
    "fieldB": [
        4
    ]
}
)");
}

TEST(SimpleCpp, FromJSONMatchesExpectedValues) {
  tangent::test::MyMessageC msg{};
  msg.parse_json(R"({
    "fieldA": [{
        "fieldA": 1,
        "fieldB": 2,
        "fieldC": 3,
        "fieldD": "MyEnumA_VALUE3"
    }],
    "fieldB": [4]
})");

  ASSERT_EQ(msg.fieldA.size(), 1);
  EXPECT_EQ(msg.fieldA[0].fieldA, 1);
  EXPECT_EQ(msg.fieldA[0].fieldB, 2.0);
  EXPECT_EQ(msg.fieldA[0].fieldC, 3);
  EXPECT_EQ(msg.fieldA[0].fieldD, tangent::test::MyEnumA_VALUE3);
  ASSERT_EQ(msg.fieldB.size(), 1);
  EXPECT_EQ(msg.fieldB[0], 4);
}
