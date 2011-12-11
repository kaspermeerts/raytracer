CC = gcc
CFLAGS = -Wall -Wextra -ggdb -std=c99 -pipe
all:

libpnglite.a: pnglite.o
	$(AR) -cq libpnglite.a pnglite.o

libpnglite.so: pnglite.o
	$(CC) -lz -shared -Wl,-soname,libpnglite.so -o libpnglite.so pnglite.o

pnglite.o: pnglite.c pnglite.h
	$(CC) $(CFLAGS) -fPIC -c pnglite.c
