include cuda.mk

CFLAGS += -fopenmp -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
OBJS += physics-cuda.o

physics-cuda.o : physics-verlet-brute-hybrid.cu
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<

nbody : $(OBJS)
	$(NVCC) $^ -Xcompiler=-fopenmp $(LDFLAGS) $(LDLIBS) -o $@
