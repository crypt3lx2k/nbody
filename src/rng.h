#ifndef RNG_H
#define RNG_H 1

/* frees underlying state */
extern void rng_free (void);

/* initializes the random number generator */
extern void rng_init (void);

/* draws a uniformly distributed number in (lower, upper] */
extern double rng_uniform (double lower, double upper);

/* draws a normally distributed number */
extern double rng_normal (double std, double mean);

#endif /* RNG_H */
