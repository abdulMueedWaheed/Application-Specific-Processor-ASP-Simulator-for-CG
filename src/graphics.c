#include "../include/graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize framebuffer
Framebuffer *fb_init(void) {
  Framebuffer *fb = (Framebuffer *)malloc(sizeof(Framebuffer));
  if (!fb)
    return NULL;

  fb->pixels = (Pixel *)calloc(FB_SIZE, sizeof(Pixel));
  if (!fb->pixels) {
    free(fb);
    return NULL;
  }

  fb->current_color = 0xFFFFFFFF; // White
  fb->draw_x = 0;
  fb->draw_y = 0;

  return fb;
}

// Free framebuffer
void fb_free(Framebuffer *fb) {
  if (fb) {
    if (fb->pixels)
      free(fb->pixels);
    free(fb);
  }
}

// Clear framebuffer to black
void fb_clear(Framebuffer *fb) {
  if (!fb || !fb->pixels)
    return;
  memset(fb->pixels, 0, FB_SIZE * sizeof(Pixel));
}

// Check if coordinates are in bounds
int fb_in_bounds(int x, int y) {
  return (x >= 0 && x < FB_WIDTH && y >= 0 && y < FB_HEIGHT);
}

// Set pixel at (x, y) with given color
void fb_set_pixel(Framebuffer *fb, int x, int y, Pixel color) {
  if (!fb || !fb->pixels)
    return;
  if (!fb_in_bounds(x, y))
    return;

  int index = y * FB_WIDTH + x;
  fb->pixels[index] = color;
}

// Get pixel at (x, y)
Pixel fb_get_pixel(Framebuffer *fb, int x, int y) {
  if (!fb || !fb->pixels)
    return 0;
  if (!fb_in_bounds(x, y))
    return 0;

  int index = y * FB_WIDTH + x;
  return fb->pixels[index];
}

// Set current drawing color
void fb_set_color(Framebuffer *fb, Pixel color) {
  if (!fb)
    return;
  fb->current_color = color;
}

// Draw pixel at current position
void fb_draw_pixel(Framebuffer *fb, int x, int y) {
  if (!fb)
    return;
  fb_set_pixel(fb, x, y, fb->current_color);
}

// Bresenham line drawing algorithm
void fb_draw_line(Framebuffer *fb, int x1, int y1, int x2, int y2) {
  if (!fb)
    return;

  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  int x = x1, y = y1;

  while (1) {
    fb_set_pixel(fb, x, y, fb->current_color);

    if (x == x2 && y == y2)
      break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }
}

// Draw line from current position by (dx, dy) offset
void fb_draw_step(Framebuffer *fb, int dx, int dy) {
  if (!fb)
    return;

  int x2 = fb->draw_x + dx;
  int y2 = fb->draw_y + dy;

  fb_draw_line(fb, fb->draw_x, fb->draw_y, x2, y2);

  fb->draw_x = x2;
  fb->draw_y = y2;
}

// Create color from RGB components
Pixel fb_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

// Create color from ARGB components
Pixel fb_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  return (a << 24) | (r << 16) | (g << 8) | b;
}

// Dump framebuffer as PPM (Portable PixMap) file
void fb_dump_ppm(Framebuffer *fb, const char *filename) {
  if (!fb || !fb->pixels)
    return;

  FILE *f = fopen(filename, "wb");
  if (!f) {
    perror("fopen");
    return;
  }

  // PPM header
  fprintf(f, "P6\n");
  fprintf(f, "%d %d\n", FB_WIDTH, FB_HEIGHT);
  fprintf(f, "255\n");

  // Write pixels (RGB, 8-bit each)
  for (int i = 0; i < FB_SIZE; i++) {
    Pixel p = fb->pixels[i];
    uint8_t r = (p >> 16) & 0xFF;
    uint8_t g = (p >> 8) & 0xFF;
    uint8_t b = p & 0xFF;

    fputc(r, f);
    fputc(g, f);
    fputc(b, f);
  }

  fclose(f);
  printf("Framebuffer dumped to %s\n", filename);
}

// Dump framebuffer as ASCII art to console
void fb_dump_ascii(Framebuffer *fb) {
  if (!fb || !fb->pixels)
    return;

  printf("\n=== Framebuffer (ASCII) ===\n");

  for (int y = 0; y < FB_HEIGHT; y += 8) {
    for (int x = 0; x < FB_WIDTH; x += 4) {
      int hit = 0;
      // Check 4x8 block
      for (int dy = 0; dy < 8 && (y + dy) < FB_HEIGHT; dy++) {
        for (int dx = 0; dx < 4 && (x + dx) < FB_WIDTH; dx++) {
          if (fb->pixels[(y + dy) * FB_WIDTH + (x + dx)] != 0) {
            hit = 1;
            break;
          }
        }
        if (hit)
          break;
      }

      if (hit) {
        printf("█");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
}
