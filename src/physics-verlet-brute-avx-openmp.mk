include physics-verlet-brute-avx.mk

OMPFLAGS = -fopenmp
CFLAGS  += $(OMPFLAGS)
LDFLAGS += $(OMPFLAGS)
