CC = gcc
DEFINES =
WARNINGS = -Wextra -Wall -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing -Wno-pointer-sign -pedantic
CFLAGS = $(WARNINGS) $(DEFINES) -std=c99 -O3 -ffast-math -pipe -ggdb -m64
SOURCES = colour.c vector.c ray.c scene.c
INCFLAGS = -I.
LDFLAGS = -lm

all: raytracer objtest

raytracer: raytracer.c $(SOURCES)
	@echo "	CC raytracer"
	@$(CC) -o raytracer raytracer.c $(CFLAGS) `xml2-config --cflags` $(LDFLAGS) `xml2-config --libs` $(INCFLAGS) $(SOURCES)

objtest: objtest.c
	@echo "	CC objtest"
	@$(CC) -o objtest $(CFLAGS) -I. objtest.c mesh.c -Lobjreader -lobjreader
ctags:
	@echo "	CTAGS"
	@ctags -R .

.PHONY: all clean ctags
