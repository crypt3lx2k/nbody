CPPFLAGS += -DVECTOR_SIZE=2 -DALIGN_BOUNDARY=16 -DALLOC_PADDING=16
CFLAGS += -msse

OBJS += physics-util.o
DEPS += physics-util.d
