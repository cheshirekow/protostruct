#include "tangent/protostruct/test/test_messages-simple.h"

#include "tangent/tjson/cpputil.h"
#include "tangent/tjson/parse.h"
#include "tangent/util/exception.h"
#include "tangent/util/hash.h"
#include "tangent/util/stringutil.h"

namespace tangent {
namespace test {

int parse_json(const std::string& json_str, MyEnumA* value) {
  switch (tangent::runtime_suci_hash(json_str)) {
    case tangent::suci_hash("MyEnumA_VALUE1"):
      *value = MyEnumA_VALUE1;
      return 0;
    case tangent::suci_hash("MyEnumA_VALUE2"):
      *value = MyEnumA_VALUE2;
      return 0;
    case tangent::suci_hash("MyEnumA_VALUE3"):
      *value = MyEnumA_VALUE3;
      return 0;
    default:
      break;
  }
  return 1;
}

tjson::OStream& operator<<(tjson::OStream& out, const MyEnumA& value) {
  switch (value) {
    case MyEnumA_VALUE1:
      out << "MyEnumA_VALUE1";
      return out;
    case MyEnumA_VALUE2:
      out << "MyEnumA_VALUE2";
      return out;
    case MyEnumA_VALUE3:
      out << "MyEnumA_VALUE3";
      return out;
    default:
      break;
  }
  return out;
}

std::string MyMessageA::to_json() {
  tjson::OSStream strm{};
  strm << *this;
  return strm.str();
}

void MyMessageA::parse_json(const std::string& json_str) {
  return tjson::parse_json(json_str, this);
}

tjson::OStream& operator<<(tjson::OStream& out, const MyMessageA& value) {
  tjson::Guard guard{&out, tjson::OBJECT};
  out << "fieldA" << value.fieldA;
  out << "fieldB" << value.fieldB;
  out << "fieldC" << value.fieldC;
  out << "fieldD" << value.fieldD;
  return out;
}

std::string MyMessageB::to_json() {
  tjson::OSStream strm{};
  strm << *this;
  return strm.str();
}

void MyMessageB::parse_json(const std::string& json_str) {
  return tjson::parse_json(json_str, this);
}

tjson::OStream& operator<<(tjson::OStream& out, const MyMessageB& value) {
  tjson::Guard guard{&out, tjson::OBJECT};
  out << "fieldA" << value.fieldA;
  return out;
}

std::string MyMessageC::to_json() {
  tjson::OSStream strm{};
  strm << *this;
  return strm.str();
}

void MyMessageC::parse_json(const std::string& json_str) {
  return tjson::parse_json(json_str, this);
}

tjson::OStream& operator<<(tjson::OStream& out, const MyMessageC& value) {
  tjson::Guard guard{&out, tjson::OBJECT};
  out << "fieldA" << value.fieldA;
  out << "fieldB" << value.fieldB;
  out << "fieldC" << value.fieldC;
  return out;
}

std::string TestFixedArray::to_json() {
  tjson::OSStream strm{};
  strm << *this;
  return strm.str();
}

void TestFixedArray::parse_json(const std::string& json_str) {
  return tjson::parse_json(json_str, this);
}

tjson::OStream& operator<<(tjson::OStream& out, const TestFixedArray& value) {
  tjson::Guard guard{&out, tjson::OBJECT};
  out << "fixedSizedArray" << value.fixedSizedArray;
  return out;
}

std::string TestAlignas::to_json() {
  tjson::OSStream strm{};
  strm << *this;
  return strm.str();
}

void TestAlignas::parse_json(const std::string& json_str) {
  return tjson::parse_json(json_str, this);
}

tjson::OStream& operator<<(tjson::OStream& out, const TestAlignas& value) {
  tjson::Guard guard{&out, tjson::OBJECT};
  out << "array" << value.array;
  return out;
}

std::string TestPrimitives::to_json() {
  tjson::OSStream strm{};
  strm << *this;
  return strm.str();
}

void TestPrimitives::parse_json(const std::string& json_str) {
  return tjson::parse_json(json_str, this);
}

tjson::OStream& operator<<(tjson::OStream& out, const TestPrimitives& value) {
  tjson::Guard guard{&out, tjson::OBJECT};
  out << "fieldA" << value.fieldA;
  out << "fieldB" << value.fieldB;
  out << "fieldC" << value.fieldC;
  out << "fieldD" << value.fieldD;
  out << "fieldE" << value.fieldE;
  out << "fieldF" << value.fieldF;
  out << "fieldG" << value.fieldG;
  out << "fieldH" << value.fieldH;
  out << "fieldI" << value.fieldI;
  out << "fieldJ" << value.fieldJ;
  out << "fieldK" << value.fieldK;
  return out;
}

}  // namespace test
}  // namespace tangent

namespace tjson {

int parse(tjson_ParseContext ctx, tangent::test::MyEnumA* value) {
  std::string str{};
  int err = tjson::parse(ctx, &str);
  if (err) {
    return err;
  }
  return tangent::test::parse_json(str, value);
}

static int MyMessageA_fielditem_callback(void* pobj, tjson_ParseContext ctx,
                                         tjson_StringPiece fieldname) {
  auto* obj = reinterpret_cast<tangent::test::MyMessageA*>(pobj);
  switch (tjson_StringPiece_suci_digest(fieldname)) {
    case tangent::suci_hash("fieldA"):
      return tjson::parse(ctx, &obj->fieldA);
    case tangent::suci_hash("fieldB"):
      return tjson::parse(ctx, &obj->fieldB);
    case tangent::suci_hash("fieldC"):
      return tjson::parse(ctx, &obj->fieldC);
    case tangent::suci_hash("fieldD"):
      return tjson::parse(ctx, &obj->fieldD);
  }
  return 0;
}

int parse(tjson_ParseContext ctx, tangent::test::MyMessageA* value) {
  return tjson_parse_object(ctx, MyMessageA_fielditem_callback, value);
}

static int MyMessageB_fielditem_callback(void* pobj, tjson_ParseContext ctx,
                                         tjson_StringPiece fieldname) {
  auto* obj = reinterpret_cast<tangent::test::MyMessageB*>(pobj);
  switch (tjson_StringPiece_suci_digest(fieldname)) {
    case tangent::suci_hash("fieldA"):
      return tjson::parse(ctx, &obj->fieldA);
  }
  return 0;
}

int parse(tjson_ParseContext ctx, tangent::test::MyMessageB* value) {
  return tjson_parse_object(ctx, MyMessageB_fielditem_callback, value);
}

static int MyMessageC_fielditem_callback(void* pobj, tjson_ParseContext ctx,
                                         tjson_StringPiece fieldname) {
  auto* obj = reinterpret_cast<tangent::test::MyMessageC*>(pobj);
  switch (tjson_StringPiece_suci_digest(fieldname)) {
    case tangent::suci_hash("fieldA"):
      return tjson::parse(ctx, &obj->fieldA);
    case tangent::suci_hash("fieldB"):
      return tjson::parse(ctx, &obj->fieldB);
    case tangent::suci_hash("fieldC"):
      return tjson::parse(ctx, &obj->fieldC);
  }
  return 0;
}

int parse(tjson_ParseContext ctx, tangent::test::MyMessageC* value) {
  return tjson_parse_object(ctx, MyMessageC_fielditem_callback, value);
}

static int TestFixedArray_fielditem_callback(void* pobj, tjson_ParseContext ctx,
                                             tjson_StringPiece fieldname) {
  auto* obj = reinterpret_cast<tangent::test::TestFixedArray*>(pobj);
  switch (tjson_StringPiece_suci_digest(fieldname)) {
    case tangent::suci_hash("fixedSizedArray"):
      return tjson::parse(ctx, &obj->fixedSizedArray);
  }
  return 0;
}

int parse(tjson_ParseContext ctx, tangent::test::TestFixedArray* value) {
  return tjson_parse_object(ctx, TestFixedArray_fielditem_callback, value);
}

static int TestAlignas_fielditem_callback(void* pobj, tjson_ParseContext ctx,
                                          tjson_StringPiece fieldname) {
  auto* obj = reinterpret_cast<tangent::test::TestAlignas*>(pobj);
  switch (tjson_StringPiece_suci_digest(fieldname)) {
    case tangent::suci_hash("array"):
      return tjson::parse(ctx, &obj->array);
  }
  return 0;
}

int parse(tjson_ParseContext ctx, tangent::test::TestAlignas* value) {
  return tjson_parse_object(ctx, TestAlignas_fielditem_callback, value);
}

static int TestPrimitives_fielditem_callback(void* pobj, tjson_ParseContext ctx,
                                             tjson_StringPiece fieldname) {
  auto* obj = reinterpret_cast<tangent::test::TestPrimitives*>(pobj);
  switch (tjson_StringPiece_suci_digest(fieldname)) {
    case tangent::suci_hash("fieldA"):
      return tjson::parse(ctx, &obj->fieldA);
    case tangent::suci_hash("fieldB"):
      return tjson::parse(ctx, &obj->fieldB);
    case tangent::suci_hash("fieldC"):
      return tjson::parse(ctx, &obj->fieldC);
    case tangent::suci_hash("fieldD"):
      return tjson::parse(ctx, &obj->fieldD);
    case tangent::suci_hash("fieldE"):
      return tjson::parse(ctx, &obj->fieldE);
    case tangent::suci_hash("fieldF"):
      return tjson::parse(ctx, &obj->fieldF);
    case tangent::suci_hash("fieldG"):
      return tjson::parse(ctx, &obj->fieldG);
    case tangent::suci_hash("fieldH"):
      return tjson::parse(ctx, &obj->fieldH);
    case tangent::suci_hash("fieldI"):
      return tjson::parse(ctx, &obj->fieldI);
    case tangent::suci_hash("fieldJ"):
      return tjson::parse(ctx, &obj->fieldJ);
    case tangent::suci_hash("fieldK"):
      return tjson::parse(ctx, &obj->fieldK);
  }
  return 0;
}

int parse(tjson_ParseContext ctx, tangent::test::TestPrimitives* value) {
  return tjson_parse_object(ctx, TestPrimitives_fielditem_callback, value);
}

}  // namespace tjson
