include cuda.mk

CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
CFLAGS += -fopenmp

LDFLAGS += -Xcompiler=-fopenmp

OBJS += physics-cuda.o
DEPS += physics-verlet-brute-hybrid.du

physics-cuda.o : physics-verlet-brute-hybrid.cu
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<
