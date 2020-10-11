CC=clang
CFLAGS=-std=c11 -c

-all: messages.o
	$(CC) -o Messages messages.o && rm messages.o lib.h.gch && ./Finished

messages.o:
	$(CC) $(CFLAGS) messages.c lib.h
