#ifndef PARTICLES_H
#define PARTICLES_H 1

#include "vector.h"

typedef struct {
  vector * x;
  vector * v;

  value * m;
  size_t n;
} particles;

#endif /* PARTICLES_H */
