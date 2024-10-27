#include "fractal_myflpt.h"
#include "swap.h"
#include "vga.h"
#include <stdio.h>
#include "cache.h"

#define OR1300

// Constants describing the output device
const int SCREEN_WIDTH = 512;   //!< screen width
const int SCREEN_HEIGHT = 512;  //!< screen height

// Constants describing the initial view port on the fractal function
const myflpt32 FRAC_WIDTH = 0x40000080u; //!< default fractal width (3.0 in Q4.28)
const myflpt32 CX_0 = 0x80000080u;      //!< default start x-coordinate (-2.0 in Q4.28)
const myflpt32 CY_0 = 0xc000007fu;      //!< default start y-coordinate (-1.5 in Q4.28)
const uint16_t N_MAX = 64;    //!< maximum number of iterations

int main() {
    printf("Starting Custom Floating Point\n");
    volatile unsigned int *vga = (unsigned int *) 0x50000020;
    volatile unsigned int reg, hi;
    rgb565 frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    myflpt32 delta = 0x40000077u;

    int i;
    vga_clear();
    printf("Starting drawing a fractal\n");
#ifdef OR1300
    /* enable the caches */
    icache_write_cfg( CACHE_DIRECT_MAPPED | CACHE_SIZE_8K | CACHE_REPLACE_FIFO );
    dcache_write_cfg( CACHE_FOUR_WAY | CACHE_SIZE_8K | CACHE_REPLACE_LRU | CACHE_WRITE_BACK );
    icache_enable(1);
    dcache_enable(1);
    printf("Caches enabled\n");
#endif
    /* Enable the vga-controller's graphic mode */
    vga[0] = swap_u32(SCREEN_WIDTH);
    vga[1] = swap_u32(SCREEN_HEIGHT);
    vga[2] = swap_u32(1);
    vga[3] = swap_u32((unsigned int) &frameBuffer[0]);
    /* Clear screen */
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) frameBuffer[i] = 0;

    draw_fractal(frameBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, &calc_mandelbrot_point_soft, &iter_to_colour, CX_0, CY_0,
                 delta, N_MAX);
#ifdef OR1300
    dcache_flush();
#endif
    printf("Done\n");
}
