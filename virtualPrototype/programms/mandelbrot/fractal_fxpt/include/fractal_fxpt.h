#ifndef FRACTAL_FXPT_H
#define FRACTAL_FXPT_H

#include <stdint.h>

// implements a fixed point type Q19.13
#define FRACTIONAL_BITS 13
typedef int32_t fxpt;
fxpt int_to_fixed(int x);
fxpt fxpt_mul(fxpt x, fxpt y);
fxpt fxpt_div(fxpt x, fxpt y);

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;

//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(fxpt cx, fxpt cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(fxpt cx, fxpt cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  fxpt cx_0, fxpt cy_0, fxpt delta, uint16_t n_max);

#endif // FRACTAL_FXPT_H
