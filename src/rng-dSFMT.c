#include <math.h>
#include <stdbool.h>

#include <sys/time.h>

#define DSFMT_MEXP 19937
#include <dSFMT.h>

#include "rng.h"

#define STORAGE_SIZE (1 * DSFMT_N64)
static double storage[STORAGE_SIZE];

static struct {
  double * base;
  double * read;
  double * end;
} array;

static bool initialized = false;

void rng_free (void) {
  return;
}

void rng_init (void) {
  struct timeval timebuf;

  if (initialized)
    return;

  (void) gettimeofday(&timebuf, NULL);
  dsfmt_gv_init_gen_rand(timebuf.tv_sec ^ timebuf.tv_usec);

  array.base = &storage[0];
  array.end  = &storage[STORAGE_SIZE-1];
  array.read = array.end + 1;

  initialized = true;
}

/*
 * Draws a uniformly distributed number
 * on the interval (0, 1].
 */
static inline double rng_array_uniform (void) {
  if (array.read > array.end) {
    dsfmt_gv_fill_array_open_close(array.base, STORAGE_SIZE);
    array.read = array.base;
  }

  return *(array.read++);
}

double rng_uniform (double lower, double upper) {
  return lower + (upper-lower)*rng_array_uniform();
}

double rng_normal (double std, double mean) {
  double x, y, r2;

  do {
    x = rng_uniform(-1, 1);
    y = rng_uniform(-1, 1);

    r2 = x*x + y*y;
  } while (r2 > 1.0);

  return std * y * sqrt(-2.0 * log(r2)/r2) + mean;
}
