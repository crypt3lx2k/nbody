#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"

static const double G = GRAVITATIONAL_CONSTANT;

static vector * a0 = NULL;
static vector * a1 = NULL;

static size_t allocated = 0;

static inline void physics_swap (void) {
  vector * t = a0;
  a0 = a1;
  a1 = t;
}

void physics_advance (particle * particles, size_t n, double dt) {
  size_t i, j;

  for (i = 0; i < n; i++) {
    particles[i].position[0] +=
      (particles[i].velocity[0] + 0.5*a0[i][0]*dt)*dt;
    particles[i].position[1] +=
      (particles[i].velocity[1] + 0.5*a0[i][1]*dt)*dt;

    a1[i][0] = 0.0;
    a1[i][1] = 0.0;
  }

  for (i = 0; i < n; i++) {
    for (j = i+1; j < n; j++) {
      double dx, dy, r, F;

      dx = particles[j].position[0] - particles[i].position[0];
      dy = particles[j].position[1] - particles[i].position[1];

      r = sqrt(dx*dx + dy*dy);
      F = G*particles[i].mass*particles[j].mass/((r*r+SOFTENING)*r);

      a1[i][0] += F * dx / particles[i].mass;
      a1[i][1] += F * dy / particles[i].mass;

      a1[j][0] -= F * dx / particles[j].mass;
      a1[j][1] -= F * dy / particles[j].mass;
    }
  }

  for (i = 0; i < n; i++) {
    particles[i].velocity[0] += 0.5*(a0[i][0]+a1[i][0])*dt;
    particles[i].velocity[1] += 0.5*(a0[i][1]+a1[i][1])*dt;
  }

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
