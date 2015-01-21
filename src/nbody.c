#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "align_malloc.h"
#include "draw.h"
#include "initial-condition.h"
#include "physics.h"

#include "nbody.h"

static size_t n;

static value * px;
static value * py;

static value * vx;
static value * vy;

static value * m;

static value dt = TIME_DELTA;

static double timer (void) {
  struct timespec now;

  (void) clock_gettime(CLOCK_MONOTONIC, &now);

  return now.tv_sec + 1e-9*now.tv_nsec;
}

static bool main_loop (void) {
  unsigned int app_state = 0;
  unsigned long int counter = 0;
  double s, t;

  initial_condition(n, px, py, vx, vy, m);

  s = 0.0;

  do {
    t = timer();
    physics_advance(dt, n, px, py, vx, vy, m);
    t = timer() - t;
    s += t;

    draw_particles(n, px, py, vx, vy, m);
    app_state = draw_input(app_state);

    if (app_state & TIME_DELTA_INCREASE)
      dt *= 2.0;
    if (app_state & TIME_DELTA_DECREASE)
      dt /= 2.0;

    app_state &= ~TIME_DELTA_INCREASE;
    app_state &= ~TIME_DELTA_DECREASE;

    counter += 1;

    if ((counter % 1000LU) == 0)
      printf("%lu\n", counter);
  } while (! (app_state & EXIT) &&
	   ! (app_state & RESET));

  printf("%lu physics iterations over %f seconds, ratio %f\n",
  	 counter, s, counter/s);

  return app_state & RESET;
}

int main (void) {
  bool restart;

  n = NUMBER_OF_PARTICLES;

  px = align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);
  py = align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);

  vx = align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);
  vy = align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);

  m  = align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);

  if (px == NULL || py == NULL || vx == NULL || vy == NULL || m == NULL) {
    perror("main");
    exit(EXIT_FAILURE);
  }

  draw_init(SCREEN_WIDTH, SCREEN_HEIGHT, FRAME_RATE);
  physics_init(n);

  do {
    restart = main_loop();
    draw_reset();
    physics_reset();
  } while (restart);

  physics_free();
  draw_free();

  align_free(m);
  align_free(vy);
  align_free(vx);
  align_free(py);
  align_free(px);

  exit(EXIT_SUCCESS);
}
