#ifndef FRACTAL_MYFLPT_H
#define FRACTAL_MYFLPT_H

#include <stdint.h>
#include "myflpt.h"
//typedef uint32_t myflpt32;
//
//#define EXPONENT_MASK 0x000000FFu
//#define MANTISSA_MASK 0x7FFFFF00u
//#define SIGN_MASK 0x80000000u
//
//#define MANTISSA_SHIFT 8

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;

//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(myflpt32 cx, myflpt32 cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(myflpt32 cx, myflpt32 cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  myflpt32 cx_0, myflpt32 cy_0, myflpt32 delta, uint16_t n_max);

#endif // FRACTAL_MYFLPT_H
