#include "physics.h"

void initial_condition (size_t n,
			value * px, value * py,
			value * vx, value * vy,
			value * m) {
  size_t i;
  value theta;
  value radius;

  theta = value_literal(0.0);
  for (i = 0; i < n; i++) {
    m[i] = value_literal(1.0);

    radius = sinv(2*theta);

    px[i] = radius*sinv(theta);
    py[i] = radius*cosv(theta);

    vx[i] = value_literal(0.0);
    vy[i] = value_literal(0.0);

    theta += 2*M_PI/n;
  }
}
