#ifndef PHYSICS_H
#define PHYSICS_H 1

#include <stddef.h>
#include "value.h"

#define GRAVITATIONAL_CONSTANT        value_literal(1.0)       /* N (m/kg)^2 */ 
#define SOFTENING                     value_literal(1e-2)      /* m */

/* advance time by dt */
extern void physics_advance (value dt, size_t n,
			     value * px, value * py,
			     value * vx, value * vy,
			     value * m);

/* frees underlying resources */
extern void physics_free (void);

/* initializes the system to handle n particles */
extern void physics_init (size_t n);

/* resets the underlying state */
extern void physics_reset (size_t n);

#endif /* PHYSICS_H */
