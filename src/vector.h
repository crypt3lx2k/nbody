#ifndef VECTOR_H
#define VECTOR_H 1

#include <math.h>

typedef float value;
#define value_literal(x) x##f

#define sqrtv(x) sqrtf((x))
#define atan2v(x, y) atan2f((x), (y))

typedef value vector __attribute__ ((vector_size(VECTOR_SIZE*sizeof(value))));

#endif /* VECTOR_H */
