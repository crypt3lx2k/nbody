#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"

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
    p->x[i][0] +=
      (p->v[i][0] + value_literal(0.5)*a0[i][0]*dt)*dt;
    p->x[i][1] +=
      (p->v[i][1] + value_literal(0.5)*a0[i][1]*dt)*dt;

    a1[i][0] = value_literal(0.0);
    a1[i][1] = value_literal(0.0);
  }

  for (i = 0; i < n; i++) {
    for (j = i+1; j < n; j++) {
      vector a, r;
      value s;

      r[0] = p->x[j][0] - p->x[i][0];
      r[1] = p->x[j][1] - p->x[i][1];

      s = (r[0]*r[0] + r[1]*r[1]) + SOFTENING*SOFTENING;
      s = s*s*s;

      a[0] = G*r[0]/sqrtv(s);
      a[1] = G*r[1]/sqrtv(s);

      a1[i][0] += a[0] * p->m[j];
      a1[i][1] += a[1] * p->m[j];

      a1[j][0] -= a[0] * p->m[i];
      a1[j][1] -= a[1] * p->m[i];
    }
  }

  for (i = 0; i < n; i++) {
    p->v[i][0] += value_literal(0.5)*(a0[i][0]+a1[i][0])*dt;
    p->v[i][1] += value_literal(0.5)*(a0[i][1]+a1[i][1])*dt;
  }

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
