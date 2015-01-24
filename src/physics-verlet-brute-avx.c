#include <immintrin.h>

#include "physics-verlet-brute-util.h"

static const value G = GRAVITATIONAL_CONSTANT;

void physics_advance (value dt, size_t n,
		      value * px, value * py,
		      value * vx, value * vy,
		      value * m) {
  size_t i, j;

  __m256 g = _mm256_set1_ps(G);
  __m256 e = _mm256_set1_ps(SOFTENING*SOFTENING);
  __m256 d = _mm256_set1_ps(dt);
  __m256 h = _mm256_set1_ps(value_literal(0.5)*dt);

#pragma omp parallel private(i, j)
  {
#pragma omp for
    for (i = 0; i < n; i += 8) {
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

      /* a1x[i] = value_literal(0.0); */
      /* a1y[i] = value_literal(0.0); */
      _mm256_store_ps(&a1x[i], _mm256_setzero_ps());
      _mm256_store_ps(&a1y[i], _mm256_setzero_ps());
    }

#pragma omp for
    for (i = 0; i < n; i += 8) {
      __m256 pxi = _mm256_load_ps(&px[i]);
      __m256 pyi = _mm256_load_ps(&py[i]);

      __m256 axi = _mm256_load_ps(&a1x[i]);
      __m256 ayi = _mm256_load_ps(&a1y[i]);

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

	/* a[0] = G*r[0]*s; */
	/* a[1] = G*r[1]*s; */
	ax = _mm256_mul_ps(_mm256_mul_ps(g, rx), s);
	ay = _mm256_mul_ps(_mm256_mul_ps(g, ry), s);

	/* a1x[i] += a[0] * m[j]; */
	/* a1y[i] += a[1] * m[j]; */
	axi = _mm256_add_ps(axi, _mm256_mul_ps(ax, mj));
	ayi = _mm256_add_ps(ayi, _mm256_mul_ps(ay, mj));
      }

      _mm256_store_ps(&a1x[i], axi);
      _mm256_store_ps(&a1y[i], ayi);
    }

#pragma omp for
    for (i = 0; i < n; i += 8) {
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
  } /* #pragma omp parallel private(i, j) */

  physics_swap();
}
