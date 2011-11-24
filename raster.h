#ifndef CG_RASTER
#define CG_RASTER

#include "colour.h"

typedef struct Raster {
	int width, height;
	Colour *buffer;
	float *zbuffer;
} Raster;

#define RASTER_PIXEL(raster, x, y) (((raster).buffer)[(raster).width * (y) + (x)])

Raster *raster_new(int width, int height);
void raster_destroy(Raster *raster);
void raster_line(Raster *raster, int x0, int y0, int x1, int y1);

#endif
