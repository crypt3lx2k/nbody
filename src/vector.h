#ifndef VECTOR_H
#define VECTOR_H 1

#include <math.h>

typedef float value;
#define value_literal(x) x##f

#define sqrtv(x) sqrtf((x))
#define atan2v(x, y) atan2f((x), (y))

typedef value v2d __attribute__ ((vector_size(2*sizeof(value))));
typedef v2d vector;

#endif /* VECTOR_H */
