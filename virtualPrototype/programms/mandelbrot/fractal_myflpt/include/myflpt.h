//
// Created by Miguel Antoons on 13/10/2024.
//

#ifndef SOC_PROGRAMMING_MYFLPT_H
#define SOC_PROGRAMMING_MYFLPT_H

#include <stdint.h>

typedef uint32_t myflpt32;

#define EXPONENT_MASK 0x000000FF
#define MANTISSA_MASK 0x7FFFFF00
#define SIGN_MASK 0x80000000

#define MANTISSA_SHIFT 8

myflpt32 assign(float initial);
myflpt32 assign(uint32_t initial);
myflpt32 add(myflpt32 number1, myflpt32 number2);
//void scale(myflpt32 numbers[], int size);
myflpt32 sub(myflpt32 number1, myflpt32 number2);
myflpt32 mul(myflpt32 number1, myflpt32 number2);
myflpt32 div(myflpt32 number1, myflpt32 number2);
myflpt32 cmp(myflpt32 number1, myflpt32 number2);

#endif //SOC_PROGRAMMING_MYFLPT_H
