CC = gcc
DEFINES =
WARNINGS = -Wextra -Wall -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing -Wno-pointer-sign -pedantic
CFLAGS = $(WARNINGS) $(DEFINES) -std=c99  -pipe -ggdb
SOURCES = colour.c glm.c matrix.c quaternion.c vector.c ray.c scene.c
INCFLAGS =
LDFLAGS = -lm

all: raytracer sdltest

raytracer: raytracer.c $(SOURCES)
	@echo "	CC raytracer"
	@$(CC) -o raytracer raytracer.c $(CFLAGS) `xml2-config --cflags` $(LDFLAGS) `xml2-config --libs` $(INCFLAGS) $(SOURCES)

sdltest: sdltest.c $(SOURCES)
	@echo "	CC sdltest"
	@$(CC) -o sdltest sdltest.c $(CFLAGS) `xml2-config --cflags` $(LDFLAGS) `xml2-config --libs` $(INCFLAGS) $(SOURCES)

ctags:
	@echo "	CTAGS"
	@ctags -R .

.PHONY: all clean ctags
