#include <math.h>

#include "physics.h"
#include "rng.h"

#include "initial-condition-solar.h"

static const value G = GRAVITATIONAL_CONSTANT;

void initial_condition (particles * p) {
  size_t i;
  size_t n = p->n;
  value M   =  value_literal(0.0);
  vector MP = {value_literal(0.0)};
  vector MV = {value_literal(0.0)};

  rng_init();

  for (i = 1; i < n; i++) {
    p->m[i] = rng_normal(MASS_STANDARD_DEVIATION,
			 MASS_EXPECTED_VALUE);

    M += p->m[i];
  }

  p->m[0] = SOLAR_MASS_RATIO*M;

  for (i = 1; i < n; i++) {
    p->x[i][0] = rng_normal(0.5, 0.0);
    p->x[i][1] = rng_normal(0.5, 0.0);
  }

  for (i = 1; i < n; i++) {
    value x, y;

    value r, angle;
    value abs_v;

    x = p->x[i][0];
    y = p->x[i][1];

    r     = sqrtv(x*x + y*y);
    angle = atan2v(x, y);

    abs_v = sqrtv(G*p->m[0]/r);

    p->v[i][0] = abs_v*cos(-angle);
    p->v[i][1] = abs_v*sin(-angle);

    MP += p->x[i]*p->m[i];
    MV += p->v[i]*p->m[i];
  }

  p->x[0] = -MP/p->m[0];
  p->v[0] = -MV/p->m[0];
}
