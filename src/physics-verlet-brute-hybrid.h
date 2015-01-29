#ifndef PHYSICS_VERLET_BRUTE_HYBRID_H
#define PHYSICS_VERLET_BRUTE_HYBRID_H 1

#include "physics.h"

#define GPU_CPU_WORK_RATIO 1.0
#define CPU_WORKSHARE (1.0/(1.0 + GPU_CPU_WORK_RATIO))
#define GPU_WORKSHARE (GPU_CPU_WORK_RATIO/(1.0 + GPU_CPU_WORK_RATIO))

#define CPU_N (n*CPU_WORKSHARE)
#define GPU_N (n-CPU_N)

extern void physics_cpu_free (void);
extern void physics_cpu_init (size_t n);
extern void physics_cpu_reset (size_t n);
extern void physics_cpu_swap (void);

extern void physics_cpu_advance_positions (value dt, size_t n,
					   const value * vx,
					   const value * vy,
					   value * px,
					   value * py);

extern void physics_cpu_calculate_forces (size_t n,
					  const value * px,
					  const value * py,
					  const value * m);

extern void physics_cpu_advance_velocities (value dt, size_t n,
					    value * vx, value * vy);

#endif /* PHYSICS_VERLET_BRUTE_HYBRID_H */
