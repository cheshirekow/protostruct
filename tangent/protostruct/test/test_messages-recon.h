#pragma once
// Generated by protostruct. DO NOT EDIT BY HAND!
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

#define FIELD_B_CAPACITY 12
#define FIELD_C_CAPACITY 10

/// This is enum "A"
typedef enum MyEnumA {
  MyEnumA_VALUE1 = 0,  //!< value 1
  MyEnumA_VALUE2 = 1,  //!< value 2
  MyEnumA_VALUE3 = 2,  //!< value 3
} MyEnumA;

/// This is message "A"
typedef struct MyMessageA {
  int32_t fieldA;   //!< field A
  double fieldB;    //!< field B
  uint64_t fieldC;  //!< field C
  MyEnumA fieldD;   //!< field D
} MyMessageA;

/// This is message "B"
typedef struct MyMessageB {
  MyMessageA fieldA;  //!< field A
} MyMessageB;

/// This is message "C"
typedef struct MyMessageC {
  MyMessageA fieldA[10];
  int32_t fieldB[FIELD_B_CAPACITY];
  int32_t fieldC[FIELD_C_CAPACITY];
} MyMessageC;

typedef struct TestFixedArray {
  double fixedSizedArray[10];
} TestFixedArray;

typedef struct TestAlignas {
  float array[4];
} TestAlignas;

typedef struct TestPrimitives {
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
} TestPrimitives;
