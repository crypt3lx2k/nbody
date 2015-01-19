#ifndef INITIAL_CONDITION_H
#define INITIAL_CONDITION_H 1

#include <stddef.h>
#include "value.h"

extern void initial_condition (size_t n,
			       value * px, value * py,
			       value * vx, value * vy,
			       value * m);

#endif /* INITIAL_CONDITION_H */
