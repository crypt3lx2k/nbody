CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY='sizeof(void *)' -DALLOC_PADDING=0

OBJS += physics-util.o
DEPS += physics-util.d
