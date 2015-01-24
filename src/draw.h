#ifndef DRAW_H
#define DRAW_H 1

#include <stddef.h>
#include "value.h"

/* user input */
enum {
  EXIT                = 1 << 0,
  RESET               = 1 << 1
};

/* frees program window */
extern void draw_free (void);

/* initializes program window */
extern void draw_init (int width, int height, int fps, size_t n);

/* handles keyboard/mouse input */
extern unsigned int draw_input (unsigned int app_state, value * dt);

/* draws particles to screen */
extern void draw_particles (value dt, size_t n,
			    const value * px, const value * py,
			    const value * vx, const value * vy,
			    const value * m);

/* */
extern void draw_reset (size_t n);

#endif /* DRAW_H */
