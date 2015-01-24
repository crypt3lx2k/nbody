#include <math.h>

#include "physics-verlet-brute-util.h"

static const value G = GRAVITATIONAL_CONSTANT;

void physics_advance (value dt, size_t n,
		      value * px, value * py,
		      value * vx, value * vy,
		      value * m) {
  size_t i, j;

#pragma omp parallel private(i, j)
  {
#pragma omp for
    for (i = 0; i < n; i++) {
      px[i] +=
	(vx[i] + value_literal(0.5)*a0x[i]*dt)*dt;
      py[i] +=
	(vy[i] + value_literal(0.5)*a0y[i]*dt)*dt;

      a1x[i] = value_literal(0.0);
      a1y[i] = value_literal(0.0);
    }

#pragma omp for
    for (i = 0; i < n; i++) {
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

	a1x[i] += a[0] * m[j];
	a1y[i] += a[1] * m[j];
      }
    }

#pragma omp for
    for (i = 0; i < n; i++) {
      vx[i] += value_literal(0.5)*(a0x[i]+a1x[i])*dt;
      vy[i] += value_literal(0.5)*(a0y[i]+a1y[i])*dt;
    }
  } /* #pragma omp parallel */

  physics_swap();
}
