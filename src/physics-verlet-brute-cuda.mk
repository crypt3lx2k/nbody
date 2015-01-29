CFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
OBJS += physics-cuda.o

NVCC = nvcc

physics-cuda.o : physics-verlet-brute-cuda.cu
	$(NVCC) -O3 -use_fast_math -Xcompiler="$(CFLAGS)" -c -o $@ $<

nbody : $(OBJS)
	$(NVCC) $^ $(LDFLAGS) $(LDLIBS) -o $@
