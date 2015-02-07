CC = gcc

NASM = nasm -f elf64
STRIP = strip -x
ECHO = echo

MV = mv
LN = ln -sf

CFLAGS  = -Ofast -march=native -Wall -Wextra
LDLIBS  = -ldSFMT -lm

OBJS = align_malloc.o draw.o initial-condition.o nbody.o physics.o rng.o

all : ../bin/nbody

include draw-flags.mk
include physics-flags.mk

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

.PHONY : clean
clean :
	$(RM) ../bin/nbody nbody *.o
