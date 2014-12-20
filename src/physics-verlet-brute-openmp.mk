include physics-verlet-brute.mk

OMPFLAGS = -fopenmp
CFLAGS  += $(OMPFLAGS)
LDFLAGS += $(OMPFLAGS)
