CC = gcc

MV = mv
LN = ln -sf

CFLAGS  = -Ofast -Wall -Wextra
LDLIBS  = -lSDL -ldSFMT -lm

include physics-flags.mk

OBJS = draw.o initial-condition.o physics.o rng.o
SRCS = $(OBJS:.o=.c)

../bin/nbody : nbody
	$(MV) nbody $@

physics-brute :
	$(LN) physics-verlet-brute.c physics.c
	$(LN) physics-verlet-brute.mk physics-flags.mk
	$(RM) physics.o
	$(MAKE)

physics-openmp :
	$(LN) physics-verlet-brute-openmp.c physics.c
	$(LN) physics-verlet-brute-openmp.mk physics-flags.mk
	$(RM) physics.o
	$(MAKE)

physics-avx :
	$(LN) physics-verlet-brute-avx.c physics.c
	$(LN) physics-verlet-brute-avx.mk physics-flags.mk
	$(RM) physics.o
	$(MAKE)

physics-openmp-avx :
	$(LN) physics-verlet-brute-avx.c physics.c
	$(LN) physics-verlet-brute-openmp-avx.mk physics-flags.mk
	$(RM) physics.o
	$(MAKE)

nbody : nbody.o $(OBJS)

.PHONY : clean
clean :
	$(RM) ../bin/nbody *.o
