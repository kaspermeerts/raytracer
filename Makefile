CC = gcc
DEFINES =
WARNINGS = -Wextra -Wall -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing -Wno-pointer-sign -pedantic
CFLAGS = $(WARNINGS) $(DEFINES) -std=c99 -O0 -ffast-math -pipe -ggdb `xml2-config --cflags`
SOURCES = colour.c vector.c scene.c ppm.c mesh.c material.c matrix.c quaternion.c
INCFLAGS = -I.
LDFLAGS = -lm -Lobjreader -lobjreader `xml2-config --libs`

all: rayviewer raytracer rasteriser matrixtest

matrixtest: matrixtest.c $(SOURCES)
	@echo "	CC matrixtest"
	@$(CC) -o matrixtest matrixtest.c $(CFLAGS) $(INCFLAGS) $(SOURCES) $(LDFLAGS) -lSDL
	
rayviewer: rayviewer.c ray.c $(SOURCES)
	@echo "	CC rayviewer"
	@$(CC) -o rayviewer rayviewer.c ray.c $(CFLAGS) $(INCFLAGS) $(SOURCES) $(LDFLAGS) -lSDL

raytracer: raytracer.c ray.c $(SOURCES)
	@echo "	CC raytracer"
	@$(CC) -o raytracer raytracer.c ray.c $(CFLAGS) $(INCFLAGS) $(SOURCES) $(LDFLAGS)

rasteriser: rasteriser.c raster.c $(SOURCES)
	@echo "	CC rasteriser"
	@$(CC) -o rasteriser rasteriser.c raster.c $(CFLAGS) $(INCFLAGS) $(SOURCES) $(LDFLAGS)

ctags:
	@echo "	CTAGS"
	@ctags -R .

.PHONY: all clean ctags
