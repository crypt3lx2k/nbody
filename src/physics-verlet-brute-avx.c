#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <immintrin.h>

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

  const __m256 g = _mm256_set1_ps(G);
  const __m256 s = _mm256_set1_ps(CUBE(SOFTENING));

#pragma omp parallel private(i, j)
  {
#pragma omp for
    for (i = 0; i < n; i++) {
      p->x[i] +=
	(p->v[i] + value_literal(0.5)*a0[i]*dt)*dt;

      a1[i][0] = value_literal(0.0);
      a1[i][1] = value_literal(0.0);
    }

#pragma omp for
    for (i = 0; i < n; i += 4) {
      __m256 xi, ai;

      xi = _mm256_loadu_ps(&p->x[i][0]);
      ai = _mm256_loadu_ps(&a1[i][0]);

      for (j = 0; j < n; j++) {
	__m256 xj;
	__m256 d;
	__m256 d2, b2;
	__m256 mj;
	__m256 r, r3, F;

	xj = _mm256_castpd_ps (
	       _mm256_broadcast_sd((double *) &p->x[j][0])
	);
	mj = _mm256_broadcast_ss(&p->m[j]);

	d = _mm256_sub_ps(xj, xi);

	/* r = _mm256_hypot_ps(d, b); */
	d2 = _mm256_mul_ps(d, d);
	b2 = _mm256_permute_ps(d2, 0b10110001);
	r = _mm256_add_ps(d2, b2);
	r = _mm256_sqrt_ps(r);

	r3 = _mm256_mul_ps(r, r);
	r3 = _mm256_mul_ps(r3, r);
	r3 = _mm256_add_ps(r3, s);

	F = _mm256_mul_ps(g, d);
	F = _mm256_div_ps(F, r3);
	F = _mm256_mul_ps(F, mj);

	ai = _mm256_add_ps(ai, F);
      }

      _mm256_storeu_ps(&a1[i][0], ai);
    }

#pragma omp for
    for (i = 0; i < n; i++)
      p->v[i] += value_literal(0.5)*(a0[i]+a1[i])*dt;
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
