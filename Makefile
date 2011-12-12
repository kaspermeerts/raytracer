CC = gcc
DEFINES =
WARNINGS = -Wextra -Wall -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing -Wno-pointer-sign -pedantic
OPTIM = -ffast-math -O0
#OPTIM = -ffast-math -O4 -flto -finline-limit=2000000000 -DNDEBUG
CFLAGS = $(WARNINGS) $(DEFINES) $(OPTIM) -std=c99 -pipe -ggdb
COMMON_SRC = colour.c vector.c quaternion.c matrix.c scene.c lighting.c ppm.c mesh.c bbox.c timer.c texture.c
RAY_SRC = ray.c shading.c $(COMMON_SRC)
RASTER_SRC = raster.c $(COMMON_SRC)
INCFLAGS = -I. `xml2-config --cflags`
LDFLAGS = -Lpnglite -lpnglite -lm -Lobjreader -lobjreader `xml2-config --libs`

#all: treetest rayviewer raytracer rasteriser
all: pnglite/libpnglite.a png2ppm raytracer

pnglite/libpnglite.a:
	$(MAKE) -C pnglite libpnglite.a

png2ppm: png2ppm.c ppm.c colour.c texture.c
	@echo "	CC png2ppm"
	@$(CC) -o png2ppm png2ppm.c ppm.c colour.c texture.c $(CFLAGS) $(INCFLAGS) $(LDFLAGS)

rayviewer: rayviewer.c $(RAY_SRC)
	@echo "	CC rayviewer"
	@$(CC) -o rayviewer rayviewer.c $(RAY_SRC) $(CFLAGS) $(INCFLAGS) $(LDFLAGS) -lSDL

raytracer: raytracer.c $(RAY_SRC)
	@echo "	CC raytracer"
	@$(CC) -o raytracer raytracer.c $(RAY_SRC) $(CFLAGS) $(INCFLAGS) $(LDFLAGS)

rasteriser: rasteriser.c $(RASTER_SRC)
	@echo "	CC rasteriser"
	@$(CC) -o rasteriser rasteriser.c $(RASTER_SRC) $(CFLAGS) $(INCFLAGS) $(LDFLAGS)

ctags:
	@echo "	CTAGS"
	@ctags -R .

.PHONY: all clean ctags
