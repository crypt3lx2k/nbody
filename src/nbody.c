#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw.h"
#include "initial-condition.h"
#include "particle.h"
#include "physics.h"

#include "nbody.h"

static particle * particles;
static size_t n_particles = NUMBER_OF_PARTICLES;

static double dt = TIME_DELTA;

static bool main_loop (void) {
  unsigned int app_state = 0;

  initial_condition(particles, n_particles);

  do {
    physics_advance(particles, n_particles, dt);
    draw_particles(particles, n_particles);
    app_state = draw_input(app_state);

    if (app_state & TIME_DELTA_INCREASE)
      dt *= 2.0;
    if (app_state & TIME_DELTA_DECREASE)
      dt /= 2.0;

    app_state &= ~TIME_DELTA_INCREASE;
    app_state &= ~TIME_DELTA_DECREASE;
  } while (! (app_state & EXIT) &&
	   ! (app_state & RESET));

  return app_state & RESET;
}

int main (void) {
  bool restart;

  particles = malloc(n_particles*sizeof(particle));

  if (particles == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }

  draw_init(SCREEN_WIDTH, SCREEN_HEIGHT, FRAME_RATE);
  physics_init(n_particles);

  do {
    restart = main_loop();
    draw_reset();
    physics_reset();
  } while (restart);

  physics_free();
  draw_free();
  free(particles);

  exit(EXIT_SUCCESS);
}
