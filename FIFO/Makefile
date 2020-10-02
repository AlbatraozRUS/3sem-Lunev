CC=clang
CFLAGS=-std=c11 -c


-all: writer.o reader.o
	$(CC) -o writer writer.o && $(CC) -o reader reader.o && rm reader.o writer.o libs.h.gch

writer.o:
	$(CC) $(CFLAGS) writer.c libs.h

reader.o:
	$(CC) $(CFLAGS) reader.c libs.h
