CC = gcc
DEFINES =
WARNINGS = -Wextra -Wall -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing -Wno-pointer-sign -pedantic
CFLAGS = $(WARNINGS) $(DEFINES) -std=c99 -O0 -ffast-math -pipe -ggdb `xml2-config --cflags`
SOURCES = colour.c vector.c ray.c scene.c ppm.c mesh.c
INCFLAGS = -I.
LDFLAGS = -lm -Lobjreader -lobjreader `xml2-config --libs`

all: raytracer objtest

raytracer: raytracer.c $(SOURCES)
	@echo "	CC raytracer"
	@$(CC) -o raytracer raytracer.c $(CFLAGS) $(INCFLAGS) $(SOURCES) $(LDFLAGS)

objtest: objtest.c
	@echo "	CC objtest"
	@$(CC) -o objtest $(CFLAGS) -I. objtest.c mesh.c -Lobjreader -lobjreader

ctags:
	@echo "	CTAGS"
	@ctags -R .

.PHONY: all clean ctags
