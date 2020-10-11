CC=clang
CFLAGS=-std=c11 -c 

-all: messages.o
	$(CC) -o Messages messages.o && rm messages.o && ./Finished

messages.o:
	$(CC) $(CFLAGS) messages.c
