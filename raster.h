#ifndef CG_RASTER
#define CG_RASTER

#include "colour.h"

typedef struct Raster {
	int width, height;
	Colour *buffer;
	float *zbuffer;
} Raster;

Raster *raster_new(int width, int height);
void raster_fill(Raster *raster, Colour c);
void raster_destroy(Raster *raster);
bool raster_pixel(Raster *raster, int x, int y, Colour c);
bool raster_z_pixel(Raster *raster, int x, int y, float z);
void raster_line(Raster *raster, int x0, int y0, int x1, int y1);

#endif
