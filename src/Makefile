CC = gcc

CAT = cat
ECHO = echo
STRIP = strip -x

MV = mv
LN = ln -sf

CFLAGS  = -Ofast -march=native -Wall -Wextra
LDLIBS  = -ldSFMT -lm

OBJS = align_malloc.o draw.o initial-condition.o nbody.o physics.o rng.o
DEPS = align_malloc.d draw.d initial-condition.d nbody.d physics.d rng.d

all : deps
	$(MAKE) ../bin/nbody

include draw-flags.mk
include physics-flags.mk

draw-opengl-sdl2 :
	$(LN) $@.c draw.c
	$(LN) $@.mk draw-flags.mk
	$(MAKE) clean
	$(MAKE)

draw-profile :
	$(LN) $@.c draw.c
	$(LN) $@.mk draw-flags.mk
	$(MAKE) clean
	$(MAKE)

draw-sdl :
	$(LN) $@.c draw.c
	$(LN) $@.mk draw-flags.mk
	$(MAKE) clean
	$(MAKE)

initial-condition-random :
	$(LN) $@.c initial-condition.c
	$(MAKE) clean
	$(MAKE)

initial-condition-solar :
	$(LN) $@.c initial-condition.c
	$(MAKE) clean
	$(MAKE)

initial-condition-polar :
	$(LN) $@.c initial-condition.c
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-openmp :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-sse :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-sse-openmp :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-avx :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-avx-asm :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-avx-openmp :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-cuda :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-verlet-brute-hybrid :
	$(LN) $@.c physics.c
	$(LN) $@.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

nbody : $(OBJS)

../bin/nbody : nbody
	$(LN) $(PWD)/$< $(PWD)/$@

deps : $(DEPS)
	$(CAT) $+ >> $@.mk

%.d : %.c
	$(CC) $(CPPFLAGS) -M $*.c -o $*.d

%.du : %.cu
	$(NVCC) $(CPPFLAGS) -M $*.cu -o $*.du

-include deps.mk

.PHONY : clean
clean :
	$(RM) ../bin/nbody nbody *.o *.d *.du deps.mk
