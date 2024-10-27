//
// Created by Miguel Antoons on 13/10/2024.
//

#include "../include/myflpt.h"

myflpt32 sub(myflpt32 number1, myflpt32 number2) {
    // first, check if the subtraction is an addition
    uint32_t sign1 = (number1 & SIGN_MASK);
    uint32_t sign2 = (number2 & SIGN_MASK);
    if (sign1 == sign2)
        sign2 = sign2 ^ SIGN_MASK;
    else if (sign2)
        return add(number1, number2 & ~SIGN_MASK);

    uint8_t exponent1 = number1 & EXPONENT_MASK;
    uint8_t exponent2 = number2 & EXPONENT_MASK;

    // scale the mantissas so that the exponents are equal
    int16_t diff_exponent = exponent1 - exponent2;
    uint8_t exponent;
    if (diff_exponent > 31)
        return number1;
    else if (diff_exponent < -31)
        return number2 ^ SIGN_MASK;

    uint32_t mantissa1 = (number1 & MANTISSA_MASK) | 0x80000000u;
    uint32_t mantissa2 = (number2 & MANTISSA_MASK) | 0x80000000u;
    if (diff_exponent < 0) {
        mantissa1 = mantissa1 >> -diff_exponent;
        exponent = exponent2;
    } else {
        mantissa2 = mantissa2 >> diff_exponent;
        exponent = exponent1;
    }

    // subtract the mantissas
    uint32_t mantissa, sign;
    if (mantissa1 > mantissa2) {
        mantissa = mantissa1 - mantissa2;
        sign = sign1;
    } else if (mantissa1 < mantissa2) {
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
    exponent = exponent - (31 - msb);

    return (sign | exponent | mantissa);
}

myflpt32 add(myflpt32 number1, myflpt32 number2) {
    // first, check if the addition is a subtraction
    uint32_t sign1 = (number1 & SIGN_MASK);
    uint32_t sign2 = (number2 & SIGN_MASK);
    if (sign1 != sign2)
        return sub(number1, number2 & ~SIGN_MASK);

    uint8_t exponent1 = (number1 & EXPONENT_MASK);
    uint8_t exponent2 = (number2 & EXPONENT_MASK);

    // scale the mantissas so that the exponents are equal
    int16_t diff_exponent = exponent1 - exponent2;
    if (diff_exponent > 31)
        return number1;
    else if (diff_exponent < -31)
        return number2;

    uint32_t mantissa1 = ((number1 & MANTISSA_MASK) | 0x80000000) >> 1;
    uint32_t mantissa2 = ((number2 & MANTISSA_MASK) | 0x80000000) >> 1;
    uint8_t exponent;
    if (diff_exponent < 0) {
        mantissa1 = mantissa1 >> -diff_exponent;
        exponent = exponent2;
    } else {
        mantissa2 = mantissa2 >> diff_exponent;
        exponent = exponent1;
    }

    // add the mantissas and adjust the exponent if needed
    mantissa1 += mantissa2;
    if (mantissa1 & 0x80000000)
        exponent++;
    else
        mantissa1 <<= 1;

    mantissa1 &= MANTISSA_MASK;

    return (sign1 | exponent | mantissa1);
}


myflpt32 mul(myflpt32 number1, myflpt32 number2) {
    if (number1 == 0 || number2 == 0)
        return 0;

    uint32_t mantissa1 = ((number1 & MANTISSA_MASK) | 0x80000000) >> 16;
    mantissa1 *= ((number2 & MANTISSA_MASK) | 0x80000000) >> 16;

    uint8_t exponent1 = number1 & EXPONENT_MASK;
    exponent1 += (number2 & EXPONENT_MASK) - 127;

    if (mantissa1 & 0x80000000)
        exponent1++;
    else
        mantissa1 <<= 1;

    mantissa1 &= MANTISSA_MASK;

    uint32_t sign1 = (number1 & SIGN_MASK) ^ (number2 & SIGN_MASK);

    return (sign1 | exponent1 | mantissa1);
}
