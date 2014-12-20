#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xmmintrin.h>

#include "align_malloc.h"
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

  __m128 g = _mm_set1_ps(G);
  __m128 e = _mm_set1_ps(SOFTENING*SOFTENING);
  __m128 h = _mm_set1_ps(value_literal(0.5)*dt);
  __m128 d = _mm_set1_ps(dt);

  for (i = 0; i < n; i += 2) {
    __m128 xi = _mm_load_ps(&p->x[i][0]);
    __m128 vi = _mm_load_ps(&p->v[i][0]);
    __m128 ai = _mm_load_ps(&a0[i][0]);
    __m128 dx;

    /* p->x[i][0] += */
    /*   (p->v[i][0] + value_literal(0.5)*a0[i][0]*dt)*dt; */
    /* p->x[i][1] += */
    /*   (p->v[i][1] + value_literal(0.5)*a0[i][1]*dt)*dt; */
    dx = _mm_mul_ps(h, ai);
    dx = _mm_add_ps(dx, vi);
    dx = _mm_mul_ps(dx, d);

    _mm_store_ps(&p->x[i][0], _mm_add_ps(xi, dx));

    /* a1[i][0] = value_literal(0.0); */
    /* a1[i][1] = value_literal(0.0); */
    _mm_store_ps(&a1[i][0], _mm_setzero_ps());
  }

  for (i = 0; i < n; i += 2) {
    __m128 xi = _mm_load_ps(&p->x[i][0]);
    __m128 ai;
    __m128 mi = _mm_loadu_ps(&p->m[i]);
    __m128 ac = _mm_setzero_ps();

    mi = _mm_unpacklo_ps(mi, mi);

    for (j = i+1; j < n; j++) {
      __m128 a, r;
      __m128 s, t;
      __m128 xj = _mm_loadu_ps(&p->x[j][0]);
      __m128 aj = _mm_loadu_ps(&a1[j][0]);
      __m128 mj = _mm_loadu_ps(&p->m[j]);

      mj = _mm_unpacklo_ps(mj, mj);

      /* r[0] = p->x[j][0] - p->x[i][0]; */
      /* r[1] = p->x[j][1] - p->x[i][1]; */
      r = _mm_sub_ps(xj, xi);

      /* s = (r[0]*r[0] + r[1]*r[1]) + SOFTENING*SOFTENING; */
      s = _mm_mul_ps(r, r);
      t = _mm_shuffle_ps(s, s, 0b10110001);
      s = _mm_add_ps(s, t);
      s = _mm_add_ps(s, e);

      /* s = s*s*s; */
      s = _mm_mul_ps(s, _mm_mul_ps(s, s));

      /* a[0] = G*r[0]/sqrtv(s); */
      /* a[1] = G*r[1]/sqrtv(s); */
      a = _mm_mul_ps (
	    _mm_mul_ps(g, r),
	    _mm_rsqrt_ps(s)
      );

      /* a1[i][0] += a[0] * p->m[j]; */
      /* a1[i][1] += a[1] * p->m[j]; */
      ac = _mm_add_ps(ac, _mm_mul_ps(a, mj));

      /* a1[j][0] -= a[0] * p->m[i]; */
      /* a1[j][1] -= a[1] * p->m[i]; */
      aj = _mm_sub_ps(aj, _mm_mul_ps(a, mi));

      _mm_storeu_ps(&a1[j][0], aj);
    }

    ai = _mm_load_ps(&a1[i][0]);
    _mm_store_ps(&a1[i][0], _mm_add_ps(ai, ac));
  }

  for (i = 0; i < n; i += 2) {
    __m128 a0i = _mm_load_ps(&a0[i][0]);
    __m128 a1i = _mm_load_ps(&a1[i][0]);
    __m128 vi  = _mm_load_ps(&p->v[i][0]);
    __m128 dv;

    /* p->v[i][0] += value_literal(0.5)*(a0[i][0]+a1[i][0])*dt; */
    /* p->v[i][1] += value_literal(0.5)*(a0[i][1]+a1[i][1])*dt; */
    dv = _mm_mul_ps(h, _mm_add_ps(a0i, a1i));

    _mm_store_ps(&p->v[i][0], _mm_add_ps(vi, dv));
  }

  physics_swap();
}

void physics_free (void) {
  align_free(a0);
  align_free(a1);

  a0 = NULL;
  a1 = NULL;
}

void physics_init (size_t n) {
  allocated = n;

  a0 = align_malloc(ALIGN_BOUNDARY, n*sizeof(vector) + ALLOC_PADDING);
  a1 = align_malloc(ALIGN_BOUNDARY, n*sizeof(vector) + ALLOC_PADDING);

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
