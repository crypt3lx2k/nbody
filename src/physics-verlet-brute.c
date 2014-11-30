#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"

#define CUBE(x) ((x)*(x)*(x))

static const value G = GRAVITATIONAL_CONSTANT;

static vector * a0 = NULL;
static vector * a1 = NULL;

static size_t allocated = 0;

static inline void physics_swap (void) {
  vector * t = a0;
  a0 = a1;
  a1 = t;
}

void physics_advance (particles * p, value dt) {
  size_t i, j;
  size_t n = p->n;

  for (i = 0; i < n; i++) {
    p->x[i] +=
      (p->v[i] + value_literal(0.5)*a0[i]*dt)*dt;

    a1[i][0] = value_literal(0.0);
    a1[i][1] = value_literal(0.0);
  }

  for (i = 0; i < n; i++) {
    for (j = i+1; j < n; j++) {
      vector d, d2;
      value  r, F;

      d  = p->x[j] - p->x[i];
      d2 = d*d;

      r = sqrtv(d2[0] + d2[1]);
      F = G*p->m[i]*p->m[j]/(r*r*r+CUBE(SOFTENING));

      a1[i] += F * d/p->m[i];
      a1[j] -= F * d/p->m[j];
    }
  }

  for (i = 0; i < n; i++)
    p->v[i] += value_literal(0.5)*(a0[i]+a1[i])*dt;

  physics_swap();
}

void physics_free (void) {
  free(a0);
  free(a1);

  a0 = NULL;
  a1 = NULL;
}

void physics_init (size_t n) {
  allocated = n;

  a0 = malloc(n*sizeof(vector));
  a1 = malloc(n*sizeof(vector));

  if (a0 == NULL || a1 == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }

  physics_reset();
}

void physics_reset (void) {
  memset(a0, 0, allocated*sizeof(vector));
  memset(a1, 0, allocated*sizeof(vector));
}
