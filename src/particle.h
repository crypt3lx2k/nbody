#ifndef PARTICLE_H
#define PARTICLE_H 1

#include "vector.h"

typedef struct {
  vector position;
  vector velocity;

  double mass;
} particle;

#endif /* PARTICLE_H */
