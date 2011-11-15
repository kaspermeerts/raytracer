CC = gcc
DEFINES =
WARNINGS = -Wextra -Wall -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing -Wno-pointer-sign -pedantic
CFLAGS = $(WARNINGS) $(DEFINES) -std=c99 -O3 -ffast-math -pipe -ggdb -m64
SOURCES = colour.c vector.c ray.c scene.c
INCFLAGS =
LDFLAGS = -lm

all: raytracer

raytracer: raytracer.c $(SOURCES)
	@echo "	CC raytracer"
	@$(CC) -o raytracer raytracer.c $(CFLAGS) `xml2-config --cflags` $(LDFLAGS) `xml2-config --libs` $(INCFLAGS) $(SOURCES)

ctags:
	@echo "	CTAGS"
	@ctags -R .

.PHONY: all clean ctags
