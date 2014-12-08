#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "align_malloc.h"

#define ALIGN_AREA 128

void * align_malloc (size_t alignment, size_t size) {
  void * p;
  uintptr_t n, r;

  if (alignment == 0 || alignment > ALIGN_AREA) {
    errno = EINVAL;
    return NULL;
  }

  p = malloc(size + ALIGN_AREA + sizeof(void *));

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
