#pragma once
// Generated by protostruct. DO NOT EDIT BY HAND!

#include <cstdint>
#include <string>
#include <vector>

#include "tangent/tjson/ostream.h"
#include "tangent/tjson/parse.h"

namespace tangent {
namespace test {

/// This is enum "A"
enum MyEnumA {
  MyEnumA_VALUE1 = 0,  //!< value 1
  MyEnumA_VALUE2 = 1,  //!< value 2
  MyEnumA_VALUE3 = 2,  //!< value 3
};

int parse_json(const std::string& json_str, MyEnumA* value);
tjson::OStream& operator<<(tjson::OStream& out, const MyEnumA& value);

/// This is message "A"
struct MyMessageA {
  int32_t fieldA;   //!< field A
  double fieldB;    //!< field B
  uint64_t fieldC;  //!< field C
  MyEnumA fieldD;   //!< field D

  void parse_json(const std::string& json_str);
  std::string to_json();
};

tjson::OStream& operator<<(tjson::OStream& out, const MyMessageA& value);

/// This is message "B"
struct MyMessageB {
  MyMessageA fieldA;  //!< field A

  void parse_json(const std::string& json_str);
  std::string to_json();
};

tjson::OStream& operator<<(tjson::OStream& out, const MyMessageB& value);

/// This is message "C"
struct MyMessageC {
  std::vector<MyMessageA> fieldA;
  std::vector<int32_t> fieldB;
  std::vector<int32_t> fieldC;

  void parse_json(const std::string& json_str);
  std::string to_json();
};

tjson::OStream& operator<<(tjson::OStream& out, const MyMessageC& value);

struct TestFixedArray {
  std::vector<double> fixedSizedArray;

  void parse_json(const std::string& json_str);
  std::string to_json();
};

tjson::OStream& operator<<(tjson::OStream& out, const TestFixedArray& value);

struct TestAlignas {
  std::vector<float> array;

  void parse_json(const std::string& json_str);
  std::string to_json();
};

tjson::OStream& operator<<(tjson::OStream& out, const TestAlignas& value);

struct TestPrimitives {
  int8_t fieldA;
  int16_t fieldB;
  int32_t fieldC;
  int64_t fieldD;
  uint8_t fieldE;
  uint16_t fieldF;
  uint32_t fieldG;
  uint64_t fieldH;
  float fieldI;
  double fieldJ;
  bool fieldK;

  void parse_json(const std::string& json_str);
  std::string to_json();
};

tjson::OStream& operator<<(tjson::OStream& out, const TestPrimitives& value);

}  // namespace test
}  // namespace tangent

namespace tjson {

int parse(tjson_ParseContext ctx, tangent::test::MyEnumA* value);

int parse(tjson_ParseContext ctx, tangent::test::MyMessageA* value);
int parse(tjson_ParseContext ctx, tangent::test::MyMessageB* value);
int parse(tjson_ParseContext ctx, tangent::test::MyMessageC* value);
int parse(tjson_ParseContext ctx, tangent::test::TestFixedArray* value);
int parse(tjson_ParseContext ctx, tangent::test::TestAlignas* value);
int parse(tjson_ParseContext ctx, tangent::test::TestPrimitives* value);

}  // namespace tjson
