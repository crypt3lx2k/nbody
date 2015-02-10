#ifndef NBODY_OPENMP_H
#define NBODY_OPENMP_H 1

#define NBODY_PRAGMA(x) _Pragma(#x)

#ifdef _OPENMP
#define NBODY_OMP_BARRIER  NBODY_PRAGMA(omp barrier)
#define NBODY_OMP_MASTER   NBODY_PRAGMA(omp master)
#define NBODY_OMP_PARALLEL NBODY_PRAGMA(omp parallel)
#else
#define NBODY_OMP_BARRIER
#define NBODY_OMP_MASTER
#define NBODY_OMP_PARALLEL
#endif /* _OPENMP */

#endif /* NBODY_OPENMP_H */
