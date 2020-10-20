CC=clang
CFLAGS=-std=c11 -c


-all: writer.o reader.o
	$(CC) -o writer writer.o && $(CC) -o reader reader.o && rm reader.o writer.o lib.h.gch

writer.o:
	$(CC) $(CFLAGS) writer.c lib.h

reader.o:
	$(CC) $(CFLAGS) reader.c lib.h
