#pragma once
// Generated by protostruct. DO NOT EDIT BY HAND!
#include <stdint.h>

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
  int32_t fieldB[12];
} MyMessageC;