#include <math.h>

#include "physics.h"
#include "rng.h"

#include "initial-condition-solar.h"

static const double G = GRAVITATIONAL_CONSTANT;

void initial_condition (particle * particles, size_t n) {
  size_t i;
  double M = 0.0;
  vector MP = {0.0};
  vector MV = {0.0};

  rng_init();

  for (i = 1; i < n; i++) {
    particles[i].mass = rng_normal(MASS_STANDARD_DEVIATION,
				   MASS_EXPECTED_VALUE);

    M += particles[i].mass;
  }

  particles[0].mass = SOLAR_MASS_RATIO*M;

  for (i = 1; i < n; i++) {
    particles[i].position[0] = rng_normal(0.5, 0.0);
    particles[i].position[1] = rng_normal(0.5, 0.0);
  }

  for (i = 1; i < n; i++) {
    double x, y;

    double r, angle;
    double abs_v;

    x = particles[i].position[0];
    y = particles[i].position[1];

    r     = sqrt(x*x + y*y);
    angle = atan2(x, y);

    abs_v = sqrt(G*particles[0].mass/r);

    particles[i].velocity[0] = abs_v*cos(-angle);
    particles[i].velocity[1] = abs_v*sin(-angle);

    MP[0] += particles[i].position[0]*particles[i].mass;
    MP[1] += particles[i].position[1]*particles[i].mass;

    MV[0] += particles[i].velocity[0]*particles[i].mass;
    MV[1] += particles[i].velocity[1]*particles[i].mass;
  }

  /* MP[0] /= M; MP[1] /= M; */
  /* MV[0] /= M; MV[1] /= M; */

  particles[0].position[0] = -MP[0]/particles[0].mass;
  particles[0].position[1] = -MP[1]/particles[0].mass;

  particles[0].velocity[0] = -MV[0]/particles[0].mass;
  particles[0].velocity[1] = -MV[1]/particles[0].mass;
}
