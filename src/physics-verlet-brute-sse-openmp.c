#include <xmmintrin.h>

#include "physics-verlet-brute-util.h"

static const value G = GRAVITATIONAL_CONSTANT;

void physics_advance (value dt, size_t n,
		      value * px, value * py,
		      value * vx, value * vy,
		      value * m) {
  size_t i, j;

  __m128 g = _mm_set1_ps(G);
  __m128 e = _mm_set1_ps(SOFTENING*SOFTENING);
  __m128 d = _mm_set1_ps(dt);
  __m128 h = _mm_set1_ps(value_literal(0.5)*dt);

#pragma omp for
  for (i = 0; i < n; i += 4) {
    __m128 dx;
    __m128 dy;

    /* px[i] += */
    /*   (vx[i] + value_literal(0.5)*a0x[i]*dt)*dt; */
    /* py[i] += */
    /*   (vy[i] + value_literal(0.5)*a0y[i]*dt)*dt; */
    dx = _mm_mul_ps(h, *(__m128 *) &a0x[i]);
    dy = _mm_mul_ps(h, *(__m128 *) &a0y[i]);

    dx = _mm_add_ps(dx, *(__m128 *) &vx[i]);
    dy = _mm_add_ps(dy, *(__m128 *) &vy[i]);

    dx = _mm_mul_ps(dx, d);
    dy = _mm_mul_ps(dy, d);

    _mm_store_ps(&px[i], _mm_add_ps(dx, *(__m128 *) &px[i]));
    _mm_store_ps(&py[i], _mm_add_ps(dy, *(__m128 *) &py[i]));
  }

#pragma omp for private(i, j)
  for (i = 0; i < n; i += 4) {
    __m128 pxi = _mm_load_ps(&px[i]);
    __m128 pyi = _mm_load_ps(&py[i]);

    __m128 axi = _mm_setzero_ps();
    __m128 ayi = _mm_setzero_ps();

    for (j = 0; j < n; j++) {
      __m128 ax, ay;
      __m128 rx, ry;
      __m128 s;

      __m128 pxj = _mm_load1_ps(&px[j]);
      __m128 pyj = _mm_load1_ps(&py[j]);

      __m128 mj = _mm_load1_ps(&m[j]);

      /* r[0] = px[j] - px[i]; */
      /* r[1] = py[j] - py[i]; */
      rx = _mm_sub_ps(pxj, pxi);
      ry = _mm_sub_ps(pyj, pyi);

      /* s = (r[0]*r[0] + r[1]*r[1]) + SOFTENING*SOFTENING; */
      s = _mm_add_ps(_mm_mul_ps(rx, rx),
		     _mm_mul_ps(ry, ry));
      s = _mm_add_ps(s, e);

      /* s = s*s*s; */
      s = _mm_mul_ps(s, _mm_mul_ps(s, s));

      /* s = value_literal(1.0)/sqrtv(s); */
      s = _mm_rsqrt_ps(s);

      /* s = s*m[j]; */
      s = _mm_mul_ps(s, mj);

      /* a[0] = G*r[0]*s; */
      /* a[1] = G*r[1]*s; */
      ax = _mm_mul_ps(_mm_mul_ps(g, rx), s);
      ay = _mm_mul_ps(_mm_mul_ps(g, ry), s);

      /* a1x[i] += a[0]; */
      /* a1y[i] += a[1]; */
      axi = _mm_add_ps(axi, ax);
      ayi = _mm_add_ps(ayi, ay);
    }

    _mm_store_ps(&a1x[i], axi);
    _mm_store_ps(&a1y[i], ayi);
  }

#pragma omp for
  for (i = 0; i < n; i += 4) {
    __m128 axi = _mm_load_ps(&a0x[i]);
    __m128 ayi = _mm_load_ps(&a0y[i]);

    __m128 dvx, dvy;
    /* vx[i] += value_literal(0.5)*(a0x[i]+a1x[i])*dt; */
    /* vy[i] += value_literal(0.5)*(a0y[i]+a1y[i])*dt; */
    dvx = _mm_mul_ps(h, _mm_add_ps(axi, *(__m128 *) &a1x[i]));
    dvy = _mm_mul_ps(h, _mm_add_ps(ayi, *(__m128 *) &a1y[i]));

    _mm_store_ps(&vx[i], _mm_add_ps(dvx, *(__m128 *) &vx[i]));
    _mm_store_ps(&vy[i], _mm_add_ps(dvy, *(__m128 *) &vy[i]));
  }

#pragma omp master
  physics_swap();
}
