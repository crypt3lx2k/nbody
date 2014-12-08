#ifndef ALIGN_MALLOC_H
#define ALIGN_MALLOC_H 1

#include <stddef.h>

/* returns allocated memory capable of holding size bytes
   aligned on an alignment sized boundary in bytes. */
extern void * align_malloc (size_t alignment, size_t size);

/* frees memory previously returned by align_malloc */
extern void align_free (void * ptr);

#endif /* ALIGN_MALLOC_H */
