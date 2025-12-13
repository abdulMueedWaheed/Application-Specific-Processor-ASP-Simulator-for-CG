#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#define FB_WIDTH  256
#define FB_HEIGHT 256
#define FB_SIZE   (FB_WIDTH * FB_HEIGHT)

typedef uint32_t Pixel;

typedef struct {
    Pixel *pixels;
    Pixel current_color;
    int draw_x;
    int draw_y;
} Framebuffer;

Framebuffer* fb_init(void);
void fb_free(Framebuffer *fb);
void fb_clear(Framebuffer *fb);
void fb_set_pixel(Framebuffer *fb, int x, int y, Pixel color);
Pixel fb_get_pixel(Framebuffer *fb, int x, int y);
void fb_set_color(Framebuffer *fb, Pixel color);
void fb_draw_pixel(Framebuffer *fb, int x, int y);
void fb_draw_line(Framebuffer *fb, int x1, int y1, int x2, int y2);
void fb_draw_step(Framebuffer *fb, int dx, int dy);
void fb_dump_ppm(Framebuffer *fb, const char *filename);
void fb_dump_ascii(Framebuffer *fb);
int fb_in_bounds(int x, int y);
Pixel fb_color_rgb(uint8_t r, uint8_t g, uint8_t b);
Pixel fb_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

#endif
