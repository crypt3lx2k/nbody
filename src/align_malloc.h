#ifndef ALIGN_MALLOC_H
#define ALIGN_MALLOC_H 1

#include <stddef.h>

extern void * align_malloc (size_t alignment, size_t size);

extern void align_free (void * ptr);

#endif /* ALIGN_MALLOC_H */
