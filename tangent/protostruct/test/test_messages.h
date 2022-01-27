// Copyright 2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#pragma once
#include <stdint.h>

/// This is enum "A"
typedef enum MyEnumA {
  MyEnumA_VALUE1,  //!< value 1
  MyEnumA_VALUE2,  //!< value 2
  MyEnumA_VALUE3,  //!< value 3
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

#define FIELD_B_CAPACITY 12
#define FIELD_C_CAPACITY 10

/// This is message "C"
typedef struct MyMessageC {
  MyMessageA fieldA[10];
  uint32_t fieldACount;
  int32_t fieldB[FIELD_B_CAPACITY];
  uint32_t fieldBCount;
  int32_t fieldC[FIELD_C_CAPACITY];
  uint32_t fieldCCount;
} MyMessageC;

/// protostruct: skip
typedef struct MyMessageD {
  int32_t fieldA;
  bool fieldB;
} MyMessageD;
