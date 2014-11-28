#ifndef PHYSICS_H
#define PHYSICS_H 1

#include <stddef.h>
#include "particle.h"

#define GRAVITATIONAL_CONSTANT        V(1.0)       /* N (m/kg)^2 */ 
#define SOFTENING                     V(1e-12)     /* m^3 */

/* advance time by dt */
extern void physics_advance (particle * particles, size_t n, value dt);

/* frees underlying resources */
extern void physics_free (void);

/* initializes the system to handle n particles */
extern void physics_init (size_t n);

/* resets the underlying state */
extern void physics_reset (void);

#endif /* PHYSICS_H */
