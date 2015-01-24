#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "align_malloc.h"

#include "physics-verlet-brute-util.h"

static size_t allocated = 0;

value * a0x = NULL;
value * a0y = NULL;

value * a1x = NULL;
value * a1y = NULL;

void physics_swap (void) {
  value * tx;
  value * ty;

  tx = a0x;
  a0x = a1x;
  a1x = tx;

  ty = a0y;
  a0y = a1y;
  a1y = ty;
}

void physics_free (void) {
  align_free(a1y);
  align_free(a1x);
  align_free(a0y);
  align_free(a0x);

  a0x = NULL;
  a0y = NULL;
  a1x = NULL;
  a1y = NULL;
}

void physics_init (size_t n) {
  allocated = n;

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

  physics_reset();
}

void physics_reset (void) {
  memset(a0x, 0, allocated*sizeof(value));
  memset(a0y, 0, allocated*sizeof(value));
  memset(a1x, 0, allocated*sizeof(value));
  memset(a1y, 0, allocated*sizeof(value));
}
