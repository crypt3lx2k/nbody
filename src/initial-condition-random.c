#include "physics.h"
#include "rng.h"

#include "initial-condition-random.h"

static const value G = GRAVITATIONAL_CONSTANT;

void initial_condition (size_t n,
			value * px, value * py,
			value * vx, value * vy,
			value * m) {
  size_t i;

  rng_init();

  for (i = 0; i < n; i++) {
    m[i] = rng_normal(MASS_STANDARD_DEVIATION,
		      MASS_EXPECTED_VALUE);
  }

  for (i = 0; i < n; i++) {
    px[i] = rng_normal(0.5, 0.0);
    py[i] = rng_normal(0.5, 0.0);
  }

  for (i = 0; i < n; i++) {
    vx[i] = rng_normal(VELOCITY_STANDARD_DEVIATION,
		       VELOCITY_EXPECTED_VALUE);
    vy[i] = rng_normal(VELOCITY_STANDARD_DEVIATION,
		       VELOCITY_EXPECTED_VALUE);
  }
}
