#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "align_malloc.h"

static inline int power_of_two (size_t p) {
  return p && !(p & (p - 1));
}

void * align_malloc (size_t alignment, size_t size) {
  void * p;
  uintptr_t n, r;

  if (!power_of_two(alignment)) {
    errno = EINVAL;
    return NULL;
  }

  /* we store the malloc'd pointer right before the
     returned pointer so that we can free later.
     in case we don't have room to store the pointer,
     make some. */
  if (alignment < sizeof(void *))
    alignment = sizeof(void *);

  p = malloc(size + alignment);

  if (p == NULL)
    return NULL;

  /* calculate offset from required alignment */
  n = (uintptr_t) p;
  r = alignment - (n % alignment);

  /* we store the malloc'd pointer right before the
     returned pointer so that we can free later.
     in case we don't have room to store the pointer
     make some. */
  if (r < sizeof(void *))
    r += alignment;

  *((void **) (n + r) - 1) = p;

  return (void *) (n + r);
}

void align_free (void * ptr) {
  void * p = *((void **) ptr - 1);
  free(p);
}
