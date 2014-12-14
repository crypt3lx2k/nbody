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

#pragma omp parallel private(i, j)
  {
#pragma omp for
    for (i = 0; i < n; i++) {
      p->x[i][0] +=
	(p->v[i][0] + value_literal(0.5)*a0[i][0]*dt)*dt;
      p->x[i][1] +=
	(p->v[i][1] + value_literal(0.5)*a0[i][1]*dt)*dt;

      a1[i][0] = value_literal(0.0);
      a1[i][1] = value_literal(0.0);
    }

#pragma omp for
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
	vector a, d;
	value r2s3;

	d[0] = p->x[j][0] - p->x[i][0];
	d[1] = p->x[j][1] - p->x[i][1];

	r2s3 = (d[0]*d[0] + d[1]*d[1]) + SOFTENING*SOFTENING;
	r2s3 = r2s3*r2s3*r2s3;

	a[0] = G*d[0]/sqrtv(r2s3);
	a[1] = G*d[1]/sqrtv(r2s3);

	a1[i][0] += a[0] * p->m[j];
	a1[i][1] += a[1] * p->m[j];
      }
    }

#pragma omp for
    for (i = 0; i < n; i++) {
      p->v[i][0] += value_literal(0.5)*(a0[i][0]+a1[i][0])*dt;
      p->v[i][1] += value_literal(0.5)*(a0[i][1]+a1[i][1])*dt;
    }
  } /* #pragma omp parallel */

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
