#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#define ALIGN_AREA 128

void * align_malloc (size_t alignment, size_t size) {
  void * p;
  uintptr_t n, r;

  if (alignment > ALIGN_AREA) {
    errno = EINVAL;
    return NULL;
  }

  p = malloc(size + ALIGN_AREA + alignment + sizeof(void *));

  if (p == NULL)
    return NULL;

  n = (uintptr_t) p;
  r = alignment - (n % alignment);

  if (r < sizeof(void *))
    r += alignment;

  *((void **) (n + r - sizeof(void *))) = p;

  return (void *) (n + r);
}

void align_free (void * ptr) {
  void * p = *((void **) (((char *) ptr) - sizeof(void *)));
  free(p);
}
