#ifndef VECTOR_H
#define VECTOR_H 1

#include <math.h>

typedef float value;
#define value_literal(x) x##f

#define sqrtv(x) sqrtf((x))
#define atan2v(x, y) atan2f((x), (y))
#define cosv(x) cosf((x))
#define sinv(x) sinf((x))

#ifdef __CUDACC__
typedef float1 value1;
typedef float2 value2;
typedef float3 value3;
typedef float4 value4;

#define rsqrtv(x) rsqrtf((x))
#endif /* __CUDA__ */

#endif /* VECTOR_H */
