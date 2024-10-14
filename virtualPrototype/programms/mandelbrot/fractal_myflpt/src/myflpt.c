//
// Created by Miguel Antoons on 13/10/2024.
//

#include "../include/myflpt.h"


myflpt32 assign(float initial) {
//    // find msb
//    int msb = 31;
//    while (!(number & (1 << msb))) msb--;
//
//    int multiplier = 1;
//    uint32_t intermediate = 0;
//    bool start = false;
//    while (msb >= 0) {
//        int divider = (1 << msb) * multiplier;
//        int temp_result = number / divider;
//        if (temp_result > 1) {
//            return 0;
//        }
//
//        if (start || temp_result == 1) {
//            start = true;
//            number -= temp_result * divider;
//            multiplier /= 2;
//            intermediate |= temp_result << msb;
//        }
//    }
    uint32_t mantissa = (initial | 0x007FFFF) << 8;
    uint32_t exponent = (initial | 0x7F800000) >> 23;
    uint32_t sign = initial | 0x80000000;
    return (sign | exponent | mantissa);
}


myflpt32 assign(uint32_t initial) {
    // find msb
    int msb = 31;
    while (!(initial & (1 << msb))) msb--;
    // shift the mantissa to the left
    uint32_t mantissa = (initial << (31 - msb)) & MANTISSA_MASK;
    // calculate the exponent
    uint8_t exponent = (31 - msb) + 127;
    uint32_t sign = 0;
    return (sign | exponent | mantissa);
}


myflpt32 add(myflpt32 number1, myflpt32 number2) {
    // first, check if the addition is a subtraction
    uint32_t sign1 = (number1 & SIGN_MASK);
    uint32_t sign2 = (number2 & SIGN_MASK);
    if (sign1 != sign2)
        return sub(number1, number2);

    uint32_t mantissa1 = ((number1 & MANTISSA_MASK) | 0x80000000);
    uint32_t mantissa2 = ((number2 & MANTISSA_MASK) | 0x80000000);

    uint8_t exponent1 = (number1 & EXPONENT_MASK);
    uint8_t exponent2 = (number2 & EXPONENT_MASK);
    // scale the mantissas so that the exponents are equal
    uint8_t diff_exponent = exponent1 - exponent2;
    if (diff_exponent > 0)
        mantissa2 = mantissa2 >> diff_exponent;
    else
        mantissa1 = mantissa1 >> diff_exponent;

    // add the mantissas and adjust the exponent if needed
    uint32_t mantissa = mantissa1 + mantissa2;
    if (mantissa & 0x80000000) {
        mantissa = mantissa >> 1;
        exponent1++;
    }

    return (sign1 | exponent1 | mantissa);
}


myflpt32 sub(myflpt32 number1, myflpt32 number2) {
    // first, check if the subtraction is an addition
    uint32_t sign1 = (number1 & SIGN_MASK);
    uint32_t sign2 = (number2 & SIGN_MASK);
    if (sign1 == sign2)
        sign2 = sign2 ^ SIGN_MASK;

    uint32_t mantissa1 = (number1 & MANTISSA_MASK) | 0x80000000;
    uint32_t mantissa2 = (number2 & MANTISSA_MASK) | 0x80000000;

    uint8_t exponent1 = number1 & EXPONENT_MASK;
    uint8_t exponent2 = number2 & EXPONENT_MASK;
    // scale the mantissas so that the exponents are equal
    uint8_t diff_exponent = exponent1 - exponent2;
    if (diff_exponent > 0)
        mantissa2 = mantissa2 >> diff_exponent;
    else
        mantissa1 = mantissa1 >> diff_exponent;

    // subtract the mantissas
    uint32_t mantissa, sign;
    if (number1 > number2) {
        mantissa = mantissa1 - mantissa2;
        sign = sign1;
    } else if (number1 < number2) {
        mantissa = mantissa2 - mantissa1;
        sign = sign2;
    } else {
        return 0;
    }

    // adjust the exponent if needed
    // find the msb of the mantissa
    int msb = 31;
    while (!(mantissa & (1 << msb))) msb--;
    mantissa = (mantissa << (31 - msb)) & MANTISSA_MASK;
    exponent1 = exponent1 - (31 - msb);

    return (sign | exponent1 | mantissa);
}


myflpt32 mul(myflpt32 number1, myflpt32 number2) {
    uint32_t sign1 = (number1 & SIGN_MASK);
    uint32_t sign2 = (number2 & SIGN_MASK);
    uint32_t sign = sign1 ^ sign2;

    uint32_t mantissa1 = (number1 & MANTISSA_MASK) | 0x80000000;
    uint32_t mantissa2 = (number2 & MANTISSA_MASK) | 0x80000000;
    uint32_t mantissa = (mantissa1 >> 8) * (mantissa2 >> 8);
    // find msb of the mantissa
    int msb = 31;
    while (!(mantissa & (1 << msb))) msb--;
    mantissa = (mantissa << (31 - msb)) & MANTISSA_MASK;
    uint8_t normalization = 8 - (31 - msb);

    uint8_t exponent1 = number1 & EXPONENT_MASK;
    uint8_t exponent2 = number2 & EXPONENT_MASK;
    uint8_t exponent = exponent1 + exponent2 - 127 + normalization;

    return (sign | exponent | mantissa);
}


myflpt32 div(myflpt32 number1, myflpt32 number2) {
    uint32_t sign1 = (number1 & SIGN_MASK);
    uint32_t sign2 = (number2 & SIGN_MASK);
    uint32_t sign = sign1 ^ sign2;

    uint32_t mantissa1 = (number1 & MANTISSA_MASK) | 0x80000000;
    uint32_t mantissa2 = (number2 & MANTISSA_MASK) | 0x80000000;
    uint32_t mantissa = mantissa1 / mantissa2;
    // find msb of the mantissa
    int msb = 31;
    while (!(mantissa & (1 << msb))) msb--;
    mantissa = (mantissa << (31 - msb)) & MANTISSA_MASK;
    uint8_t normalization = 31 - msb;

    uint8_t exponent1 = number1 & EXPONENT_MASK;
    uint8_t exponent2 = number2 & EXPONENT_MASK;
    uint8_t exponent = exponent1 - exponent2 - 127 - normalization;

    return (sign | exponent | mantissa);
}


int8_t cmp(myflpt32 number1, myflpt32 number2) {
    uint8_t sign1 = (number1 & SIGN_MASK);
    uint8_t sign2 = (number2 & SIGN_MASK);
    if (sign1 != sign2)
        return sign2 - sign1;

    uint8_t exponent1 = number1 & EXPONENT_MASK;
    uint8_t exponent2 = number2 & EXPONENT_MASK;
    if (exponent1 != exponent2)
        return exponent1 - exponent2;

    uint32_t mantissa1 = (number1 & MANTISSA_MASK) | 0x80000000;
    uint32_t mantissa2 = (number2 & MANTISSA_MASK) | 0x80000000;
    if (mantissa1 != mantissa2)
        return mantissa1 - mantissa2;

    return 0;
}
