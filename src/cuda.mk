NVCC = nvcc
NVCCBIN = -ccbin=$(shell which $(CC))
NVCCFLAGS = -O3 -use_fast_math $(CPPFLAGS) $(NVCCBIN) -Xcompiler="$(CFLAGS)"

# linking must be done with nvcc
nbody :
	$(NVCC) $+ $(NVCCBIN) $(LDFLAGS) $(LDLIBS) -o $@
