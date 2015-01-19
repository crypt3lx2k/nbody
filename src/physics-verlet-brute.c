#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"

static const value G = GRAVITATIONAL_CONSTANT;

static value * a0x = NULL;
static value * a0y = NULL;

static value * a1x = NULL;
static value * a1y = NULL;

static size_t allocated = 0;

static inline void physics_swap (void) {
  value * tx;
  value * ty;

  tx = a0x;
  a0x = a1x;
  a1x = tx;

  ty = a0y;
  a0y = a1y;
  a1y = ty;
}

void physics_advance (value dt, size_t n,
		      value * px, value * py,
		      value * vx, value * vy,
		      value * m) {
  size_t i, j;

  for (i = 0; i < n; i++) {
    px[i] +=
      (vx[i] + value_literal(0.5)*a0x[i]*dt)*dt;
    py[i] +=
      (vy[i] + value_literal(0.5)*a0y[i]*dt)*dt;

    a1x[i] = value_literal(0.0);
    a1y[i] = value_literal(0.0);
  }

  for (i = 0; i < n; i++) {
    for (j = i+1; j < n; j++) {
      value a[2], r[2];
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

      a1x[j] -= a[0] * m[i];
      a1y[j] -= a[1] * m[i];
    }
  }

  for (i = 0; i < n; i++) {
    vx[i] += value_literal(0.5)*(a0x[i]+a1x[i])*dt;
    vy[i] += value_literal(0.5)*(a0y[i]+a1y[i])*dt;
  }

  physics_swap();
}

void physics_free (void) {
  free(a1y);
  free(a1x);
  free(a0y);
  free(a0x);

  a0x = NULL;
  a0y = NULL;
  a1x = NULL;
  a1y = NULL;
}

void physics_init (size_t n) {
  allocated = n;

  a0x = malloc(n*sizeof(value));
  a0y = malloc(n*sizeof(value));
  a1x = malloc(n*sizeof(value));
  a1y = malloc(n*sizeof(value));

  if (a0x == NULL || a0y == NULL || a1x == NULL || a1y == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }

  physics_reset();
}

void physics_reset (void) {
  memset(a0x, 0, allocated*sizeof(value));
  memset(a0y, 0, allocated*sizeof(value));
  memset(a1x, 0, allocated*sizeof(value));
  memset(a1y, 0, allocated*sizeof(value));
}
