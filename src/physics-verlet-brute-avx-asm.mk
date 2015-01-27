CFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32 -mavx

OBJS += physics-asm.o physics-util.o

physics-asm.o : physics-verlet-brute-avx-asm.s
	$(NASM) $< -o $@
	$(STRIP) $@
