#ifndef PHYSICS_H
#define PHYSICS_H 1

#include <stddef.h>
#include "particles.h"

#define GRAVITATIONAL_CONSTANT        value_literal(1.0)       /* N (m/kg)^2 */ 
#define SOFTENING                     value_literal(1e-3)      /* m */

/* advance time by dt */
extern void physics_advance (particles * p, value dt);

/* frees underlying resources */
extern void physics_free (void);

/* initializes the system to handle n particles */
extern void physics_init (size_t n);

/* resets the underlying state */
extern void physics_reset (void);

#endif /* PHYSICS_H */
