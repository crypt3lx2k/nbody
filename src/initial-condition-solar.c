#include <math.h>

#include "physics.h"
#include "rng.h"

#include "initial-condition-solar.h"

static const value G = GRAVITATIONAL_CONSTANT;

void initial_condition (size_t n,
			value * px, value * py,
			value * vx, value * vy,
			value * m) {
  size_t i;
  value M = value_literal(0.0);
  value MP[VECTOR_SIZE] = {value_literal(0.0)};
  value MV[VECTOR_SIZE] = {value_literal(0.0)};

  rng_init();

  for (i = 1; i < n; i++) {
    m[i] = rng_normal(MASS_STANDARD_DEVIATION,
			 MASS_EXPECTED_VALUE);

    M += m[i];
  }

  m[0] = SOLAR_MASS_RATIO*M;

  for (i = 1; i < n; i++) {
    px[i] = rng_normal(0.5, 0.0);
    py[i] = rng_normal(0.5, 0.0);
  }

  for (i = 1; i < n; i++) {
    value x, y;

    value r, angle;
    value abs_v;

    x = px[i];
    y = py[i];

    r     = sqrtv(x*x + y*y);
    angle = atan2v(x, y);

    abs_v = sqrtv(G*m[0]/r);

    vx[i] = abs_v*cosv(-angle);
    vy[i] = abs_v*sinv(-angle);

    MP[0] += px[i]*m[i];
    MP[1] += py[i]*m[i];

    MV[0] += vx[i]*m[i];
    MV[1] += vy[i]*m[i];
  }

  /* set sun so that center/velocity of mass is 0,0 */
  px[0] = -MP[0]/m[0];
  py[0] = -MP[1]/m[0];

  vx[0] = -MV[0]/m[0];
  vy[0] = -MV[1]/m[0];
}
