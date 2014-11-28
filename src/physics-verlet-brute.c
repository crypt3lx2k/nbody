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

void physics_advance (particle * particles, size_t n, value dt) {
  size_t i, j;

  for (i = 0; i < n; i++) {
    particles[i].position +=
      (particles[i].velocity + V(0.5)*a0[i]*dt)*dt;

    a1[i][0] = V(0.0);
    a1[i][1] = V(0.0);
  }

  for (i = 0; i < n; i++) {
    for (j = i+1; j < n; j++) {
      vector d, d2;
      value r, F;

      d  = particles[j].position - particles[i].position;
      d2 = d*d;

      r = sqrtv(d2[0] + d2[1]);
      F = G*particles[i].mass*particles[j].mass/(r*r*r+SOFTENING);

      a1[i] += F * d/particles[i].mass;
      a1[j] -= F * d/particles[j].mass;
    }
  }

  for (i = 0; i < n; i++)
    particles[i].velocity += V(0.5)*(a0[i]+a1[i])*dt;

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
