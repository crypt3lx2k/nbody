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

physics-brute :
	$(LN) physics-verlet-brute.c physics.c
	$(LN) physics-verlet-brute.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-brute-openmp :
	$(LN) physics-verlet-brute-openmp.c physics.c
	$(LN) physics-verlet-brute-openmp.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-sse :
	$(LN) physics-verlet-brute-sse.c physics.c
	$(LN) physics-verlet-brute-sse.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-sse-openmp :
	$(LN) physics-verlet-brute-sse-openmp.c physics.c
	$(LN) physics-verlet-brute-sse-openmp.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-avx :
	$(LN) physics-verlet-brute-avx.c physics.c
	$(LN) physics-verlet-brute-avx.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-avx-asm :
	$(LN) physics-verlet-brute-avx-asm.c physics.c
	$(LN) physics-verlet-brute-avx-asm.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-avx-openmp :
	$(LN) physics-verlet-brute-avx.c physics.c
	$(LN) physics-verlet-brute-avx-openmp.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-cuda :
	$(LN) physics-verlet-brute-cuda.c physics.c
	$(LN) physics-verlet-brute-cuda.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-hybrid :
	$(LN) physics-verlet-brute-hybrid.c physics.c
	$(LN) physics-verlet-brute-hybrid.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

initial-condition-random :
	$(LN) initial-condition-random.c initial-condition.c
	$(MAKE) clean
	$(MAKE)

initial-condition-solar :
	$(LN) initial-condition-solar.c initial-condition.c
	$(MAKE) clean
	$(MAKE)

nbody : $(OBJS)

../bin/nbody : nbody
	$(LN) $(PWD)/$< $(PWD)/$@

.PHONY : clean
clean :
	$(RM) ../bin/nbody nbody *.o
