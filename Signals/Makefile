CC=clang
CFLAGS=-std=c11 -c

-all: Signal.o
	$(CC) -o Signal Signal.o && rm Signal.o lib.h.gch && ./Finished

Signal.o:
	$(CC) $(CFLAGS) signal.c lib.h
