CFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32

NVCC = nvcc

physics-util.o :
	$(ECHO) | $(CC) -xc -c -o $@ -

physics.o : physics-verlet-brute-cuda.cu
	$(NVCC) -O3 -use_fast_math -c -o $@ $<

nbody : $(OBJS)
	$(NVCC) $^ $(LDFLAGS) $(LDLIBS) -o $@
