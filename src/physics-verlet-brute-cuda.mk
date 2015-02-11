include cuda.mk

CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32

OBJS += physics-verlet-brute-cuda.o
DEPS += physics-verlet-brute-cuda.du

physics-verlet-brute-cuda.o : physics-verlet-brute-cuda.cu
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<
