CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=32 -DALLOC_PADDING=32
CFLAGS += -mavx -Wno-unknown-pragmas

OBJS += physics-util.o
DEPS += physics-util.d
