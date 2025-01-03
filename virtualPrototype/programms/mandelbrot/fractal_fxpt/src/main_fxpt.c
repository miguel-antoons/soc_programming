#include "cache.h"
#include "fractal_fxpt.h"
#include "swap.h"
#include "vga.h"
#include "cache.h"
#include <stddef.h>
#include <stdio.h>

#define OR1300

// Constants describing the output device
const int SCREEN_WIDTH = 512;  //!< screen width
const int SCREEN_HEIGHT = 512; //!< screen height

// Constants describing the initial view port on the fractal function
const fxpt FRAC_WIDTH = 0x3 << FRACTIONAL_BITS;         //!< default fractal width (3.0 in Q19.13)
const fxpt CX_0 = 0xfffffffe << FRACTIONAL_BITS;        //!< default start x-coordinate (-2.0 in Q19.13)
const fxpt CY_0 = 0xffffffe8 << (FRACTIONAL_BITS - 4);  //!< default start y-coordinate (-1.5 in Q19.13)
const uint16_t N_MAX = 64;                              //!< maximum number of iterations

int main() {
    printf("Starting Fixed Point\n");
    volatile unsigned int *vga = (unsigned int *)0x50000020;
    volatile unsigned int reg, hi;
    rgb565 frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    fxpt delta = fxpt_div(FRAC_WIDTH, int_to_fixed(SCREEN_WIDTH));
    int i;
    vga_clear();
    printf("Starting drawing a fractal\n");
#ifdef OR1300
    /* enable the caches */
    icache_write_cfg(CACHE_DIRECT_MAPPED | CACHE_SIZE_8K | CACHE_REPLACE_FIFO);
    dcache_write_cfg(CACHE_FOUR_WAY | CACHE_SIZE_8K | CACHE_REPLACE_LRU |
                   CACHE_WRITE_BACK);
    icache_enable(1);
    dcache_enable(1);
    printf("Caches enabled\n");
#endif
    /* Enable the vga-controller's graphic mode */
    vga[0] = swap_u32(SCREEN_WIDTH);
    vga[1] = swap_u32(SCREEN_HEIGHT);
    vga[2] = swap_u32(1);
    vga[3] = swap_u32((unsigned int)&frameBuffer[0]);
    /* Clear screen */
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        frameBuffer[i] = 0;

    draw_fractal(frameBuffer, SCREEN_WIDTH, SCREEN_HEIGHT,
                &calc_mandelbrot_point_soft, &iter_to_colour,
                CX_0, CY_0, delta, N_MAX);
#ifdef OR1300
    dcache_flush();
#endif
    printf("Done\n");
}
