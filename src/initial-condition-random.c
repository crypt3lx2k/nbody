#include "physics.h"
#include "rng.h"

#include "initial-condition-random.h"

static const value G = GRAVITATIONAL_CONSTANT;

void initial_condition (size_t n,
			value * px, value * py,
			value * vx, value * vy,
			value * m) {
  size_t i, j;
  value M = value_literal(0.0);

  for (i = 0; i < n; i++) {
    m[i] = rng_normal(MASS_STANDARD_DEVIATION,
		      MASS_EXPECTED_VALUE);

    M += m[i];
  }

  for (i = 0; i < n; i++) {
    px[i] = rng_normal(1.0, 0.0);
    py[i] = rng_normal(1.0, 0.0);
  }

  for (i = 0; i < n; i++) {
    value x, y;
    value d[VECTOR_SIZE] = {value_literal(0.0)};
    value p[VECTOR_SIZE] = {value_literal(0.0)};

    value rad, angle;
    value abs_v;

    for (j = 0; j < n; j++) {
      value a[VECTOR_SIZE], r[VECTOR_SIZE];
      value s;

      r[0] = px[j] - px[i];
      r[1] = py[j] - py[i];

      s = (r[0]*r[0] + r[1]*r[1]) + SOFTENING*SOFTENING;
      s = s*s*s;
      s = value_literal(1.0)/sqrtv(s);

      a[0] = G*r[0]*s;
      a[1] = G*r[1]*s;

      d[0] += a[0] * m[j];
      d[1] += a[1] * m[j];

      p[0] += r[0] * m[j];
      p[1] += r[1] * m[j];
    }

    x = p[0]/M;
    y = p[1]/M;

    rad   = sqrtv(x*x + y*y);
    angle = atan2v(y, x);

    rad *= sqrtv(d[0]*d[0] + d[1]*d[1]);
    abs_v = sqrtv(rad);

    vx[i] = abs_v*cosv(angle - M_PI_2);
    vy[i] = abs_v*sinv(angle - M_PI_2);
  }
}
