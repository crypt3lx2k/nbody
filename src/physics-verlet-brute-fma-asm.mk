NASM = nasm

CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
CFLAGS += -mavx

OBJS += physics-asm.o physics-util.o
DEPS += physics-util.d

physics-asm.o : physics-verlet-brute-fma-asm.s
	$(NASM) -f elf64 $< -o $@
	$(STRIP) $@
