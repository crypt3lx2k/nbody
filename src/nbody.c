#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include "draw.h"
#include "initial-condition.h"
#include "particles.h"
#include "physics.h"

#include "nbody.h"

static particles p;
static value dt = TIME_DELTA;

static double timer (void) {
  struct timeval t;

  (void) gettimeofday(&t, NULL);

  return t.tv_sec + 1e-6*t.tv_usec;
}

static bool main_loop (void) {
  unsigned int app_state = 0;
  unsigned long int counter = 0;
  double s, t;

  initial_condition(&p);

  s = 0.0;

  do {
    t = timer();
    physics_advance(&p, dt);
    t = timer() - t;
    s += t;

    draw_particles(&p);
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

  p.n = NUMBER_OF_PARTICLES;
  p.x = malloc(p.n * sizeof(vector) + 256);
  p.v = malloc(p.n * sizeof(vector) + 256);
  p.m = malloc(p.n * sizeof(value) + 256);

  if (p.x == NULL || p.v == NULL || p.m == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }

  draw_init(SCREEN_WIDTH, SCREEN_HEIGHT, FRAME_RATE);
  physics_init(p.n);

  do {
    restart = main_loop();
    draw_reset();
    physics_reset();
  } while (restart);

  physics_free();
  draw_free();

  free(p.x);
  free(p.v);
  free(p.m);

  exit(EXIT_SUCCESS);
}
