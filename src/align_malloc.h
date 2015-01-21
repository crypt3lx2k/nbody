#ifndef ALIGN_MALLOC_H
#define ALIGN_MALLOC_H 1

#include <stddef.h>

/* frees memory previously returned by align_malloc */
extern void align_free (void * ptr);

/* returns allocated memory capable of holding size bytes
   aligned on an alignment sized boundary in bytes. */
extern void * align_malloc (size_t alignment, size_t size);

/* returns allocated memory capable of holding size bytes
   aligned on an alignment sized boundary in bytes. Also
   adds zeroed padding at the end of the array. The rest
   of the array is not zeroed out. */
extern void * align_padded_malloc (size_t alignment, size_t size, size_t padding);

#endif /* ALIGN_MALLOC_H */
