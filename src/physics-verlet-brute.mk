# apparently GCC does much better code generation for
# 4xfloat vectors. Even if half of the computations
# are wasted.
CFLAGS += -DVECTOR_SIZE=4
