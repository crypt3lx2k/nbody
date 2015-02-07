NVCC = nvcc
NVCCFLAGS = -O3 -use_fast_math $(CPPFLAGS) -Xcompiler="$(CFLAGS)"

# linking must be done with nvcc
nbody :
	$(NVCC) $+ $(LDFLAGS) $(LDLIBS) -o $@
