#ifndef DRAW_H
#define DRAW_H 1

#include <stddef.h>
#include "value.h"

/* user input */
enum {
  EXIT                = 1 << 0,
  RESET               = 1 << 1,
  TIME_DELTA_INCREASE = 1 << 2,
  TIME_DELTA_DECREASE = 1 << 3
};

/* frees program window */
extern void draw_free (void);

/* initializes program window */
extern void draw_init (int width, int height, int fps);

/* handles keyboard/mouse input */
extern unsigned int draw_input (unsigned int app_state);

/* draws particles to screen */
extern void draw_particles (size_t n,
			    const value * px, const value * py,
			    const value * vx, const value * vy,
			    const value * m);

/* */
extern void draw_reset (void);

#endif /* DRAW_H */
