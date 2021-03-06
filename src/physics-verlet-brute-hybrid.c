#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <immintrin.h>

#include "align_malloc.h"

#include "physics-verlet-brute-hybrid.h"

static const value G = GRAVITATIONAL_CONSTANT;

static value * a0x = NULL;
static value * a0y = NULL;

static value * a1x = NULL;
static value * a1y = NULL;

void physics_cpu_advance_positions (value dt, size_t n,
				    const value * vx, const value * vy,
				    value * px, value * py) {
  size_t i;
  size_t cpu_n = CPU_N;

  __m256 d = _mm256_set1_ps(dt);
  __m256 h = _mm256_set1_ps(value_literal(0.5)*dt);

#pragma omp for
  for (i = 0; i < cpu_n; i += 8) {
    __m256 dx;
    __m256 dy;

    /* px[i] += */
    /*   (vx[i] + value_literal(0.5)*a0x[i]*dt)*dt; */
    /* py[i] += */
    /*   (vy[i] + value_literal(0.5)*a0y[i]*dt)*dt; */
    dx = _mm256_mul_ps(h, *(__m256 *) &a0x[i]);
    dy = _mm256_mul_ps(h, *(__m256 *) &a0y[i]);

    dx = _mm256_add_ps(dx, *(__m256 *) &vx[i]);
    dy = _mm256_add_ps(dy, *(__m256 *) &vy[i]);

    dx = _mm256_mul_ps(dx, d);
    dy = _mm256_mul_ps(dy, d);

    _mm256_store_ps(&px[i], _mm256_add_ps(dx, *(__m256 *) &px[i]));
    _mm256_store_ps(&py[i], _mm256_add_ps(dy, *(__m256 *) &py[i]));
  }
}

void physics_cpu_calculate_forces (size_t n,
				   const value * px, const value * py,
				   const value * m) {
  size_t i, j;
  size_t cpu_n = CPU_N;

  __m256 g = _mm256_set1_ps(G);
  __m256 e = _mm256_set1_ps(SOFTENING*SOFTENING);

#pragma omp for private(i, j)
  for (i = 0; i < cpu_n; i += 8) {
    __m256 pxi = _mm256_load_ps(&px[i]);
    __m256 pyi = _mm256_load_ps(&py[i]);

    __m256 axi = _mm256_setzero_ps();
    __m256 ayi = _mm256_setzero_ps();

    for (j = 0; j < n; j++) {
      __m256 ax, ay;
      __m256 rx, ry;
      __m256 s;

      __m256 pxj = _mm256_broadcast_ss(&px[j]);
      __m256 pyj = _mm256_broadcast_ss(&py[j]);

      __m256 mj = _mm256_broadcast_ss(&m[j]);

      /* r[0] = px[j] - px[i]; */
      /* r[1] = py[j] - py[i]; */
      rx = _mm256_sub_ps(pxj, pxi);
      ry = _mm256_sub_ps(pyj, pyi);

      /* s = (r[0]*r[0] + r[1]*r[1]) + SOFTENING*SOFTENING; */
      s = _mm256_add_ps(_mm256_mul_ps(rx, rx),
			_mm256_mul_ps(ry, ry));
      s = _mm256_add_ps(s, e);

      /* s = s*s*s; */
      s = _mm256_mul_ps(s, _mm256_mul_ps(s, s));

      /* s = value_literal(1.0)/sqrtv(s); */
      s = _mm256_rsqrt_ps(s);

      /* s = s*m[j]; */
      s = _mm256_mul_ps(s, mj);

      /* a[0] = G*r[0]*s; */
      /* a[1] = G*r[1]*s; */
      ax = _mm256_mul_ps(_mm256_mul_ps(g, rx), s);
      ay = _mm256_mul_ps(_mm256_mul_ps(g, ry), s);

      /* a1x[i] += a[0]; */
      /* a1y[i] += a[1]; */
      axi = _mm256_add_ps(axi, ax);
      ayi = _mm256_add_ps(ayi, ay);
    }

    _mm256_store_ps(&a1x[i], axi);
    _mm256_store_ps(&a1y[i], ayi);
  }
}

void physics_cpu_advance_velocities (value dt, size_t n,
				     value * vx, value * vy) {
  size_t i;
  size_t cpu_n = CPU_N;

  __m256 h = _mm256_set1_ps(value_literal(0.5)*dt);

#pragma omp for
  for (i = 0; i < cpu_n; i += 8) {
    __m256 axi = _mm256_load_ps(&a0x[i]);
    __m256 ayi = _mm256_load_ps(&a0y[i]);

    __m256 dvx, dvy;
    /* vx[i] += value_literal(0.5)*(a0x[i]+a1x[i])*dt; */
    /* vy[i] += value_literal(0.5)*(a0y[i]+a1y[i])*dt; */
    dvx = _mm256_mul_ps(h, _mm256_add_ps(axi, *(__m256 *) &a1x[i]));
    dvy = _mm256_mul_ps(h, _mm256_add_ps(ayi, *(__m256 *) &a1y[i]));

    _mm256_store_ps(&vx[i], _mm256_add_ps(dvx, *(__m256 *) &vx[i]));
    _mm256_store_ps(&vy[i], _mm256_add_ps(dvy, *(__m256 *) &vy[i]));
  }
}

void physics_cpu_swap (void) {
  value * tx;
  value * ty;

  tx = a0x;
  a0x = a1x;
  a1x = tx;

  ty = a0y;
  a0y = a1y;
  a1y = ty;
}

void physics_cpu_free (void) {
  align_free(a1y);
  align_free(a1x);
  align_free(a0y);
  align_free(a0x);

  a0x = NULL;
  a0y = NULL;
  a1x = NULL;
  a1y = NULL;
}

void physics_cpu_init (size_t n) {
  a0x =
    align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);
  a0y =
    align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);
  a1x =
    align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);
  a1y =
    align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(value), ALLOC_PADDING);

  if (a0x == NULL || a0y == NULL || a1x == NULL || a1y == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }
}

void physics_cpu_reset (size_t n) {
  memset(a0x, 0, n*sizeof(value));
  memset(a0y, 0, n*sizeof(value));
  memset(a1x, 0, n*sizeof(value));
  memset(a1y, 0, n*sizeof(value));
}
