// Generated by protostruct. DO NOT EDIT BY HAND!

#include <stdint.h>

#include "tangent/protostruct/test/test_messages.pb.h"
#include "tangent/protostruct/test/test_messages.pbwire.h"

MyEnumA pb2c(const tangent::test::MyEnumA& proto) {
  switch (proto) {
    case tangent::test::MyEnumA_VALUE1:
      return MyEnumA_VALUE1;
    case tangent::test::MyEnumA_VALUE2:
      return MyEnumA_VALUE2;
    case tangent::test::MyEnumA_VALUE3:
      return MyEnumA_VALUE3;
    default:
      break;
  }
  return MyEnumA_VALUE1;
}

tangent::test::MyEnumA c2pb(MyEnumA value) {
  switch (value) {
    case MyEnumA_VALUE1:
      return tangent::test::MyEnumA_VALUE1;
    case MyEnumA_VALUE2:
      return tangent::test::MyEnumA_VALUE2;
    case MyEnumA_VALUE3:
      return tangent::test::MyEnumA_VALUE3;
  }

  return tangent::test::MyEnumA_VALUE1;
}
void pb2c(const tangent::test::MyMessageA& proto, MyMessageA* cobj) {
  cobj->fieldA = proto.fielda();
  cobj->fieldB = proto.fieldb();
  cobj->fieldC = proto.fieldc();
  cobj->fieldD = pb2c(proto.fieldd());
}

void c2pb(const MyMessageA& cobj, tangent::test::MyMessageA* proto) {
  proto->set_fielda(cobj.fieldA);
  proto->set_fieldb(cobj.fieldB);
  proto->set_fieldc(cobj.fieldC);
  proto->set_fieldd(c2pb(cobj.fieldD));
}

void pb2c(const tangent::test::MyMessageB& proto, MyMessageB* cobj) {
  pb2c(proto.fielda(), &cobj->fieldA);
}

void c2pb(const MyMessageB& cobj, tangent::test::MyMessageB* proto) {
  c2pb(cobj.fieldA, proto->mutable_fielda());
}

void pb2c(const tangent::test::MyMessageC& proto, MyMessageC* cobj) {
  cobj->fieldACount = std::min<int>(proto.fielda_size(), 10);
  for (int idx = 0; idx < static_cast<int>(cobj->fieldACount); idx++) {
    pb2c(proto.fielda(idx), &cobj->fieldA[idx]);
  }
  cobj->fieldBCount = std::min<int>(proto.fieldb_size(), 12);
  for (int idx = 0; idx < static_cast<int>(cobj->fieldBCount); idx++) {
    cobj->fieldB[idx] = proto.fieldb(idx);
  }
}

void c2pb(const MyMessageC& cobj, tangent::test::MyMessageC* proto) {
  proto->mutable_fielda()->Clear();
  proto->mutable_fielda()->Reserve(cobj.fieldACount);
  for (int idx = 0; idx < static_cast<int>(cobj.fieldACount); idx++) {
    c2pb(cobj.fieldA[idx], proto->add_fielda());
  }
  proto->mutable_fieldb()->Clear();
  proto->mutable_fieldb()->Reserve(cobj.fieldBCount);
  for (int idx = 0; idx < static_cast<int>(cobj.fieldBCount); idx++) {
    proto->add_fieldb(cobj.fieldB[idx]);
  }
}
