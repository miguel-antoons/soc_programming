#include "fractal_fxpt.h"
#include <stdint.h>
#include <stdio.h>
#include <swap.h>

const fxpt TWO = 0x2 << FRACTIONAL_BITS;
const fxpt FOUR = 0x4 << FRACTIONAL_BITS;

inline fxpt float_to_fixed(float x) {
    return (fxpt)(x * FRACTIONAL_SHIFT);
}

inline fxpt int_to_fixed(int x) {
    return x << FRACTIONAL_BITS;
}

inline float fixed_to_float(fxpt x) {
    return ((float)x) / FRACTIONAL_SHIFT;
}

inline fxpt fxpt_mul(fxpt x, fxpt y) {
    return (fxpt)((x * y) >> FRACTIONAL_BITS);
}

inline fxpt fxpt_div(fxpt x, fxpt y) {
    return (fxpt)((x << FRACTIONAL_BITS) / y);
}

//! \brief  Mandelbrot fractal point calculation function
//! \param  cx    x-coordinate
//! \param  cy    y-coordinate
//! \param  n_max maximum number of iterations
//! \return       number of performed iterations at coordinate (cx, cy)
uint16_t calc_mandelbrot_point_soft(fxpt cx, fxpt cy, uint16_t n_max) {
    fxpt x = cx;
    fxpt y = cy;
    uint16_t n = 0;
    fxpt xx, yy, two_xy;
    do {
        xx = fxpt_mul(x, x);
        yy = fxpt_mul(y, y);
        two_xy = fxpt_mul(fxpt_mul(TWO, x), y);

        x = xx - yy + cx;
        y = two_xy + cy;
        ++n;
    } while ((xx + yy < FOUR) && (n < n_max));
    return n;
}


//! \brief  Map number of performed iterations to black and white
//! \param  iter  performed number of iterations
//! \param  n_max maximum number of iterations
//! \return       colour
rgb565 iter_to_bw(uint16_t iter, uint16_t n_max) {
    if (iter == n_max) {
        return 0x0000;
    }
    return 0xffff;
}


//! \brief  Map number of performed iterations to grayscale
//! \param  iter  performed number of iterations
//! \param  n_max maximum number of iterations
//! \return       colour
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max) {
    if (iter == n_max) {
        return 0x0000;
    }
    uint16_t brightness = iter & 0xf;
    return swap_u16(((brightness << 12) | ((brightness << 7) | brightness << 1)));
}


//! \brief Calculate binary logarithm for unsigned integer argument x
//! \note  For x equal 0, the function returns -1.
int ilog2(unsigned x) {
    if (x == 0) return -1;
    int n = 1;
    if ((x >> 16) == 0) {
        n += 16;
        x <<= 16;
    }
    if ((x >> 24) == 0) {
        n += 8;
        x <<= 8;
    }
    if ((x >> 28) == 0) {
        n += 4;
        x <<= 4;
    }
    if ((x >> 30) == 0) {
        n += 2;
        x <<= 2;
    }
    n -= x >> 31;
    return 31 - n;
}


//! \brief  Map number of performed iterations to a colour
//! \param  iter  performed number of iterations
//! \param  n_max maximum number of iterations
//! \return colour in rgb565 format little Endian (big Endian for openrisc)
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max) {
    if (iter == n_max) {
        return 0x0000;
    }
    uint16_t brightness = (iter & 1) << 4 | 0xF;
    uint16_t r = (iter & (1 << 3)) ? brightness : 0x0;
    uint16_t g = (iter & (1 << 2)) ? brightness : 0x0;
    uint16_t b = (iter & (1 << 1)) ? brightness : 0x0;
    return swap_u16(((r & 0x1f) << 11) | ((g & 0x1f) << 6) | ((b & 0x1f)));
}

rgb565 iter_to_colour1(uint16_t iter, uint16_t n_max) {
    if (iter == n_max) {
        return 0x0000;
    }
    uint16_t brightness = ((iter & 0x78) >> 2) ^ 0x1F;
    uint16_t r = (iter & (1 << 2)) ? brightness : 0x0;
    uint16_t g = (iter & (1 << 1)) ? brightness : 0x0;
    uint16_t b = (iter & (1 << 0)) ? brightness : 0x0;
    return swap_u16(((r & 0xf) << 12) | ((g & 0xf) << 7) | ((b & 0xf) << 1));
}

//! \brief  Draw fractal into frame buffer
//! \param  width  width of frame buffer
//! \param  height height of frame buffer
//! \param  cfp_p  pointer to fractal function
//! \param  i2c_p  pointer to function mapping number of iterations to colour
//! \param  cx_0   start x-coordinate
//! \param  cy_0   start y-coordinate
//! \param  delta  increment for x- and y-coordinate
//! \param  n_max  maximum number of iterations
void draw_fractal(rgb565 *fbuf, int width, int height, calc_frac_point_p cfp_p,
                  iter_to_colour_p i2c_p, fxpt cx_0, fxpt cy_0,
                  fxpt delta, uint16_t n_max) {
    rgb565 *pixel = fbuf;
    fxpt cy = cy_0;
    for (int k = 0; k < height; ++k) {
        fxpt cx = cx_0;
        for (int i = 0; i < width; ++i) {
            uint16_t n_iter = (*cfp_p)(cx, cy, n_max);
            rgb565 colour = (*i2c_p)(n_iter, n_max);
            *(pixel++) = colour;
            cx += delta;
        }
        cy += delta;
    }
}
