draw.o : draw.c
	$(CC) $(CFLAGS) -Wno-unused-parameter -c -o $@ $<
