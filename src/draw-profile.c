#include "draw.h"

static unsigned int counter = 0;

void draw_free (void) {
}

void draw_init (int width, int height, int fps, size_t n) {
}

unsigned int draw_input (unsigned int app_state, value * dt) {
  return EXIT;
}

void draw_particles (value dt, size_t n,
		     const value * px, const value * py,
		     const value * vx, const value * vy,
		     const value * m) {
}

int draw_redraw (void) {
  counter += 1;
  return counter >= 100;
}

void draw_reset (size_t n) {
}
