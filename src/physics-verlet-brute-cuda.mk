include cuda.mk

CFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
OBJS += physics-cuda.o

physics-cuda.o : physics-verlet-brute-cuda.cu
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<

nbody : $(OBJS)
	$(NVCC) $^ $(LDFLAGS) $(LDLIBS) -o $@
