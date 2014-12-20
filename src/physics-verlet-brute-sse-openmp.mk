include physics-verlet-brute-sse.mk

OMPFLAGS = -fopenmp
CFLAGS  += $(OMPFLAGS)
LDFLAGS += $(OMPFLAGS)
