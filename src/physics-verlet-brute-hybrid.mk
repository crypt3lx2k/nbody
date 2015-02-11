include cuda.mk

CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
CFLAGS += -fopenmp

LDFLAGS += -Xcompiler=-fopenmp

OBJS += physics-verlet-brute-hybrid.o 
DEPS += physics-verlet-brute-hybrid.du

physics-verlet-brute-hybrid.o : physics-verlet-brute-hybrid.cu
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<
