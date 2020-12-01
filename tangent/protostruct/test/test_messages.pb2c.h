#pragma once
// Generated by protostruct. DO NOT EDIT BY HAND!

#include "tangent/protostruct/pbwire.h"
#include "tangent/protostruct/test/test_messages.h"
#include "tangent/protostruct/test/test_messages.pb.h"

// Convert an enum value from a C++ binding object to a MyEnumA native
// structure
MyEnumA pb2c(const tangent::test::MyEnumA& proto);
// Copy data from a C++ binding object to a MyMessageA native structure
void pb2c(const tangent::test::MyMessageA& proto, MyMessageA* obj);
// Copy data from a C++ binding object to a MyMessageB native structure
void pb2c(const tangent::test::MyMessageB& proto, MyMessageB* obj);
// Copy data from a C++ binding object to a MyMessageC native structure
void pb2c(const tangent::test::MyMessageC& proto, MyMessageC* obj);

// Convert a native MyEnumA value to a protobuf C++ binding value
tangent::test::MyEnumA c2pb(MyEnumA value);
// Copy a MyMessageA native structure to a protobuf C++ binding object
int c2pb(const MyMessageA& obj, tangent::test::MyMessageA* proto);
// Copy a MyMessageB native structure to a protobuf C++ binding object
int c2pb(const MyMessageB& obj, tangent::test::MyMessageB* proto);
// Copy a MyMessageC native structure to a protobuf C++ binding object
int c2pb(const MyMessageC& obj, tangent::test::MyMessageC* proto);
