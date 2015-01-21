#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "align_malloc.h"

static inline int power_of_two (size_t p) {
  return p && !(p & (p - 1));
}

void align_free (void * ptr) {
  void * p;

  if (ptr == NULL)
    return;

  p = *((void **) ptr - 1);
  free(p);
}

void * align_malloc (size_t alignment, size_t size) {
  return align_padded_malloc(alignment, size, 0);
}

void * align_padded_malloc (size_t alignment, size_t size, size_t padding) {
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

  p = malloc(alignment + size + padding);

  if (p == NULL)
    return NULL;

  /* calculate offset from required alignment */
  n = (uintptr_t) p;
  r = alignment - (n % alignment);

  if (r < sizeof(void *))
    r += alignment;

  *((void **) (n + r) - 1) = p;
  memset((char *) (n + r) + size, 0x0, padding);

  return (void *) (n + r);
}
